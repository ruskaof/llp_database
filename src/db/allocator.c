//
// Created by ruskaof on 2/10/23.
//

#include "allocator.h"
#include "file.h"

#include "../utils/logging.h"

int
init_empty_file_with_element(int fd, uint64_t element_size, enum ElementType element_type, uint64_t *element_offset) {
    logger(LL_DEBUG, __func__, "Initializing empty file.");

    int change_file_size_result = change_file_size(fd, ALLOC_SIZE);
    if (change_file_size_result == -1) {
        return -1;
    }

    void *file_data_pointer;

    int mmap_result = mmap_file(fd, &file_data_pointer, 0, ALLOC_SIZE);
    if (mmap_result == -1) {
        return -1;
    }

    struct FileHeader *file_header = (struct FileHeader *) file_data_pointer;

    file_header->last_element_offset = FIRST_ELEMENT_OFFSET;
    file_header->has_deleted_elements = false;
    file_header->has_table_metadata_elements = false;
    file_header->has_table_data_elements = false;
    switch (element_type) {
        case ET_DELETED:
            file_header->has_deleted_elements = true;
            file_header->first_deleted_element_offset = FIRST_ELEMENT_OFFSET;
            break;
        case ET_TABLE_METADATA:
            file_header->has_table_metadata_elements = true;
            file_header->last_table_metadata_element_offset = FIRST_ELEMENT_OFFSET;
            break;
        case ET_TABLE_DATA:
            file_header->has_table_data_elements = true;
            file_header->last_table_data_element_offset = FIRST_ELEMENT_OFFSET;
            break;
    }

    struct ElementHeader *element_header = (struct ElementHeader *) ((char *) file_data_pointer + FIRST_ELEMENT_OFFSET);
    element_header->element_type = element_type;
    element_header->element_size = element_size;
    element_header->has_next_element_of_type = false;
    element_header->has_prev_element_of_type = false;
    element_header->has_prev_element = false;

    *element_offset = FIRST_ELEMENT_OFFSET;

    int munmap_result = munmap_file(file_data_pointer, ALLOC_SIZE, fd);
    if (munmap_result == -1) {
        return -1;
    }

    return 0;
}

int find_suitable_deleted_element_offset(void *file_data_pointer, uint64_t requested_element_size,
                                         uint64_t *suitable_deleted_element_offset) {
    logger(LL_DEBUG, __func__, "Finding suitable deleted element of size %ld.", requested_element_size);

    struct FileHeader *file_header = (struct FileHeader *) file_data_pointer;

    if (!file_header->has_deleted_elements) {
        logger(LL_DEBUG, __func__, "File has no deleted elements.");
        return -1;
    }

    uint64_t current_offset = file_header->first_deleted_element_offset;

    while (current_offset <= file_header->last_element_offset) {
        struct ElementHeader *element_header = (struct ElementHeader *) ((char *) file_data_pointer + current_offset);

        if (element_header->element_size >= requested_element_size) {
            logger(LL_DEBUG, __func__, "Found suitable deleted element of size %ld.", element_header->element_size);
            *suitable_deleted_element_offset = current_offset;
            return 0;
        }

        if (!element_header->has_next_element_of_type) {
            logger(LL_DEBUG, __func__, "Could not find suitable deleted element.");
            return -1;
        }

        current_offset = element_header->next_element_of_type_offset;
    }

    logger(LL_ERROR, __func__, "Cannot get to this point. CHECK THIS.");
    return -1;
}

void set_offsets_to_deleted_element(const void *file_data_pointer, uint64_t split_element_offset,
                                    bool has_next_element_of_type, uint64_t next_element_of_type_offset,
                                    bool has_prev_element_of_type, uint64_t prev_element_of_type_offset) {
    struct ElementHeader *split_element_header =
        (struct ElementHeader *) ((char *) file_data_pointer + split_element_offset);

    split_element_header->has_next_element_of_type = has_next_element_of_type;
    split_element_header->next_element_of_type_offset = next_element_of_type_offset;
    split_element_header->has_prev_element_of_type = has_prev_element_of_type;
    split_element_header->prev_element_of_type_offset = prev_element_of_type_offset;

    if (has_next_element_of_type) {
        struct ElementHeader *next_element_header = (struct ElementHeader *) ((char *) file_data_pointer +
                                                                              next_element_of_type_offset);

        next_element_header->has_prev_element_of_type = true;
        next_element_header->prev_element_of_type_offset = split_element_offset;
    }

    if (has_prev_element_of_type) {
        struct ElementHeader *prev_element_header = (struct ElementHeader *) ((char *) file_data_pointer +
                                                                              prev_element_of_type_offset);

        prev_element_header->has_next_element_of_type = true;
        prev_element_header->next_element_of_type_offset = split_element_offset;
    }
}

int split_deleted_element_for_allocation(void *file_data_pointer, uint64_t deleted_element_offset,
                                         uint64_t requested_element_size) {
    struct ElementHeader *deleted_element_header = (struct ElementHeader *) ((char *) file_data_pointer +
                                                                             deleted_element_offset);

    uint64_t original_element_size = deleted_element_header->element_size;
    uint64_t old_prev_element_offset = deleted_element_header->prev_element_offset;
    uint64_t old_has_prev_element = deleted_element_header->has_prev_element;

    if (original_element_size < requested_element_size ||
        original_element_size - requested_element_size < MIN_ELEMENT_SIZE) {
        logger(LL_DEBUG, __func__, "Deleted element is too small to be splitted.");
        return -1;
    }

    bool old_has_next_element_of_type = deleted_element_header->has_next_element_of_type;
    uint64_t old_next_element_of_type_offset = deleted_element_header->next_element_of_type_offset;
    bool old_has_prev_element_of_type = deleted_element_header->has_prev_element_of_type;
    uint64_t old_prev_element_of_type_offset = deleted_element_header->prev_element_of_type_offset;

    uint64_t split_element_offset = deleted_element_offset + requested_element_size;

    struct ElementHeader *split_element_header = (struct ElementHeader *) ((char *) file_data_pointer +
                                                                           split_element_offset);
    split_element_header->element_type = ET_DELETED;
    split_element_header->prev_element_offset = deleted_element_offset;
    split_element_header->has_prev_element = true;

    set_offsets_to_deleted_element(file_data_pointer, split_element_offset, old_has_next_element_of_type,
                                   old_next_element_of_type_offset, old_has_prev_element_of_type,
                                   old_prev_element_of_type_offset);


    deleted_element_header->element_size = requested_element_size;
    deleted_element_header->prev_element_of_type_offset = old_prev_element_offset;
    deleted_element_header->has_prev_element = old_has_prev_element;

    return 0;
}

void init_new_element_offsets_for_table_metadata(const void *file_data_pointer,
                                                 const uint64_t new_element_offset,
                                                 struct FileHeader *file_header,
                                                 struct ElementHeader *allocated_element_header) {
    if (!file_header->has_table_metadata_elements) {
        file_header->has_table_metadata_elements = true;
        file_header->last_table_metadata_element_offset = new_element_offset;
    } else {
        struct ElementHeader *last_table_metadata_element_header = (struct ElementHeader *) (
            (char *) file_data_pointer + file_header->last_table_metadata_element_offset);
        last_table_metadata_element_header->has_next_element_of_type = true;
        last_table_metadata_element_header->next_element_of_type_offset = new_element_offset;
        allocated_element_header->has_prev_element_of_type = true;
        allocated_element_header->prev_element_of_type_offset = file_header->last_table_metadata_element_offset;
    }

    file_header->last_table_metadata_element_offset = new_element_offset;
}

void init_new_element_offsets_for_table_data(const void *file_data_pointer,
                                             const uint64_t new_element_offset,
                                             struct FileHeader *file_header,
                                             struct ElementHeader *allocated_element_header) {
    if (!file_header->has_table_data_elements) {
        file_header->has_table_data_elements = true;
        file_header->last_table_data_element_offset = new_element_offset;
    } else {
        struct ElementHeader *last_table_data_element_header = (struct ElementHeader *) ((char *) file_data_pointer +
                                                                                         file_header->last_table_data_element_offset);
        last_table_data_element_header->has_next_element_of_type = true;
        last_table_data_element_header->next_element_of_type_offset = new_element_offset;
        allocated_element_header->has_prev_element_of_type = true;
        allocated_element_header->prev_element_of_type_offset = file_header->last_table_data_element_offset;
    }

    file_header->last_table_data_element_offset = new_element_offset;
}

int allocate_element(int fd, uint64_t requested_element_size, enum ElementType element_type, uint64_t *element_offset) {
    logger(LL_DEBUG, __func__, "Allocating element of type %d with size %ld.", element_type, requested_element_size);

    uint64_t file_size = get_file_size(fd);
    if (file_size == -1) {
        return -1;
    }

    if (requested_element_size < MIN_ELEMENT_SIZE) {
        requested_element_size = MIN_ELEMENT_SIZE;
    }

    if (file_size == 0) {
        return init_empty_file_with_element(fd, requested_element_size, element_type, element_offset);
    }

    void *file_data_pointer;

    int mmap_result = mmap_file(fd, &file_data_pointer, 0, file_size);
    if (mmap_result == -1) {
        return -1;
    }

    uint64_t suitable_deleted_element_offset;
    int find_suitable_deleted_element_offset_result = find_suitable_deleted_element_offset(file_data_pointer,
                                                                                           requested_element_size,
                                                                                           &suitable_deleted_element_offset);

    struct FileHeader *file_header = (struct FileHeader *) file_data_pointer;

    if (find_suitable_deleted_element_offset_result == 0) {
        logger(LL_DEBUG, __func__, "Found suitable deleted page of size %ld.", requested_element_size);
        split_deleted_element_for_allocation(file_data_pointer, suitable_deleted_element_offset,
                                             requested_element_size);

        *element_offset = suitable_deleted_element_offset;

        struct ElementHeader *allocated_element_header = (struct ElementHeader *) ((char *) file_data_pointer +
                                                                                   suitable_deleted_element_offset);
        allocated_element_header->element_size = requested_element_size;
        allocated_element_header->element_type = element_type;

        switch (element_type) {
            case ET_DELETED:
                logger(LL_ERROR, __func__, "Cannot allocate deleted element.");
                return -1;
            case ET_TABLE_METADATA:
                init_new_element_offsets_for_table_metadata(file_data_pointer, *element_offset,
                                                            file_header, allocated_element_header);
                break;
            case ET_TABLE_DATA:
                init_new_element_offsets_for_table_data(file_data_pointer, *element_offset, file_header,
                                                        allocated_element_header);
                break;
        }

        int munmap_result = munmap_file(file_data_pointer, file_size, fd);
        if (munmap_result == -1) {
            return -1;
        }
        return 0;
    }

    logger(LL_DEBUG, __func__, "Could not find suitable deleted page of size %ld.", requested_element_size);

    struct ElementHeader *last_element_header = (struct ElementHeader *) ((char *) file_data_pointer +
                                                                          file_header->last_element_offset);

    if (file_size - (file_header->last_element_offset + last_element_header->element_size) < requested_element_size) {
        logger(LL_DEBUG, __func__, "File size is not enough to allocate new element.");
        int change_file_size_result = change_file_size(fd, file_size + ALLOC_SIZE);
        if (change_file_size_result == -1) {
            return -1;
        }

        file_size += ALLOC_SIZE;

        int munmap_result = munmap_file(file_data_pointer, file_size, fd);
        if (munmap_result == -1) {
            return -1;
        }

        mmap_result = mmap_file(fd, &file_data_pointer, 0, file_size);
        if (mmap_result == -1) {
            return -1;
        }
    }

    *element_offset = file_header->last_element_offset + last_element_header->element_size;

    struct ElementHeader *allocated_element_header = (struct ElementHeader *) ((char *) file_data_pointer +
                                                                               *element_offset);

    allocated_element_header->element_size = requested_element_size;
    allocated_element_header->element_type = element_type;
    allocated_element_header->has_next_element_of_type = false;

    allocated_element_header->has_prev_element = true;
    allocated_element_header->prev_element_offset = file_header->last_element_offset;


    switch (element_type) {
        case ET_DELETED:
            logger(LL_ERROR, __func__, "Cannot allocate deleted element.");
            return -1;
        case ET_TABLE_METADATA:
            init_new_element_offsets_for_table_metadata(file_data_pointer, *element_offset, file_header,
                                                        allocated_element_header);
            break;
        case ET_TABLE_DATA:
            init_new_element_offsets_for_table_data(file_data_pointer, *element_offset, file_header,
                                                    allocated_element_header);
            break;
    }

    file_header->last_element_offset += last_element_header->element_size;

    int munmap_result = munmap_file(file_data_pointer, file_size, fd);
    if (munmap_result == -1) {
        return -1;
    }

    return 0;
}

void reset_deleted_element_typed_neighbors_offset(void *file_data_pointer, uint64_t element_offset) {
    struct ElementHeader *element_header = (struct ElementHeader *) ((char *) file_data_pointer + element_offset);

    if (element_header->has_prev_element_of_type) {
        struct ElementHeader *prev_element_header = (struct ElementHeader *) ((char *) file_data_pointer +
                                                                              element_header->prev_element_of_type_offset);
        if (element_header->has_next_element_of_type) {
            prev_element_header->has_next_element_of_type = true;
            prev_element_header->next_element_of_type_offset = element_header->next_element_of_type_offset;
        } else {
            prev_element_header->has_next_element_of_type = false;
        }
    }

    if (element_header->has_next_element_of_type) {
        struct ElementHeader *next_element_header = (struct ElementHeader *) ((char *) file_data_pointer +
                                                                              element_header->next_element_of_type_offset);
        next_element_header->has_prev_element = true;
        next_element_header->prev_element_offset = element_header->prev_element_offset;

        if (element_header->has_prev_element_of_type) {
            next_element_header->has_prev_element_of_type = true;
            next_element_header->prev_element_of_type_offset = element_header->prev_element_of_type_offset;
        } else {
            next_element_header->has_prev_element_of_type = false;
        }
    }

    struct FileHeader *file_header = (struct FileHeader *) file_data_pointer;
    if (file_header->first_deleted_element_offset == element_offset) {
        if (element_header->has_next_element_of_type) {
            file_header->first_deleted_element_offset = element_header->next_element_of_type_offset;
        } else {
            file_header->has_deleted_elements = false;
        }
    }
}

void merge_deleted_element(void *file_data_pointer, uint64_t element_offset) {
    logger(LL_DEBUG, __func__, "Merging deleted element at offset %ld.", element_offset);

    struct FileHeader *file_header = (struct FileHeader *) file_data_pointer;
    struct ElementHeader *element_header = (struct ElementHeader *) ((char *) file_data_pointer + element_offset);

    if (file_header->last_element_offset != element_offset) {
        uint64_t next_element_offset = element_offset + element_header->element_size;
        struct ElementHeader *next_element_header = (struct ElementHeader *) ((char *) file_data_pointer +
                                                                              next_element_offset);
        if (next_element_header->element_type == ET_DELETED) {
            logger(LL_DEBUG, __func__, "Next element is also deleted. Merging.");
            element_header->element_size += next_element_header->element_size;
            reset_deleted_element_typed_neighbors_offset(file_data_pointer, next_element_offset);
        }
    }

    if (element_offset != FIRST_ELEMENT_OFFSET) {
        uint64_t prev_element_offset = element_header->prev_element_offset;
        struct ElementHeader *prev_element_header = (struct ElementHeader *) ((char *) file_data_pointer +
                                                                              prev_element_offset);
        if (prev_element_header->element_type == ET_DELETED) {
            logger(LL_DEBUG, __func__, "Previous element is also deleted. Merging.");
            prev_element_header->element_size += element_header->element_size;
        } else {
            reset_deleted_element_typed_neighbors_offset(file_data_pointer, element_offset);
        }
    } else {
        reset_deleted_element_typed_neighbors_offset(file_data_pointer, element_offset);
    }
}

int delete_element(int fd, uint64_t element_offset) {
    logger(LL_DEBUG, __func__, "Deleting element at offset %ld.", element_offset);

    uint64_t file_size = get_file_size(fd);
    if (file_size == 0) {
        return -1;
    }

    void *file_data_pointer;

    int mmap_result = mmap_file(fd, &file_data_pointer, 0, file_size);
    if (mmap_result == -1) {
        return -1;
    }

    struct FileHeader *file_header = (struct FileHeader *) file_data_pointer;
    struct ElementHeader *element_header = (struct ElementHeader *) ((char *) file_data_pointer + element_offset);

    switch (element_header->element_type) {
        case ET_DELETED:
            logger(LL_DEBUG, __func__, "Element is already deleted.");
            return -1;
        case ET_TABLE_METADATA:
            if (file_header->has_table_metadata_elements &&
                file_header->last_table_metadata_element_offset == element_offset) {
                if (element_header->has_prev_element_of_type) {
                    file_header->last_table_metadata_element_offset = element_header->prev_element_of_type_offset;
                } else {
                    file_header->has_table_metadata_elements = false;
                }
            }
            break;
        case ET_TABLE_DATA:
            if (file_header->has_table_data_elements && file_header->last_table_data_element_offset == element_offset) {
                if (element_header->has_prev_element_of_type) {
                    file_header->last_table_data_element_offset = element_header->prev_element_of_type_offset;
                } else {
                    file_header->has_table_data_elements = false;
                }
            }
            break;
    }


    element_header->element_type = ET_DELETED;

    if (file_header->has_deleted_elements) {
        struct ElementHeader *first_deleted_element_header =
            (struct ElementHeader *) ((char *) file_data_pointer + file_header->first_deleted_element_offset);
        first_deleted_element_header->prev_element_of_type_offset = element_offset;
        first_deleted_element_header->has_prev_element_of_type = true;
        element_header->next_element_of_type_offset = file_header->first_deleted_element_offset;
        element_header->has_next_element_of_type = true;
        element_header->has_prev_element_of_type = false;
    } else {
        file_header->has_deleted_elements = true;
        file_header->first_deleted_element_offset = element_offset;
        element_header->has_next_element_of_type = false;
        element_header->has_prev_element_of_type = false;
    }

    merge_deleted_element(file_data_pointer, element_offset);

    int munmap_result = munmap_file(file_data_pointer, file_size, fd);
    if (munmap_result == -1) {
        return -1;
    }

    return 0;
}
