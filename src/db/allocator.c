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
            file_header->last_deleted_element_offset = FIRST_ELEMENT_OFFSET;
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
    element_header->has_prev_element_of_type = false;

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

    struct ElementHeader *deleted_element_header =
        (struct ElementHeader *) ((char *) file_data_pointer + file_header->last_deleted_element_offset);

    if (deleted_element_header->element_size >= requested_element_size) {
        *suitable_deleted_element_offset = file_header->last_deleted_element_offset;
        return 0;
    }

    while (deleted_element_header->has_prev_element_of_type) {
        deleted_element_header =
            (struct ElementHeader *) ((char *) file_data_pointer + deleted_element_header->prev_element_of_type_offset);
        if (deleted_element_header->element_size >= requested_element_size) {
            *suitable_deleted_element_offset = deleted_element_header->prev_element_of_type_offset;
            return 0;
        }
    }

    logger(LL_DEBUG, __func__, "Could not find suitable deleted element.");
    return -1;
}

void erase_neighbors_data_about_element(void *file_data_pinter, uint64_t element_offset) {
    struct FileHeader *file_header = (struct FileHeader *) file_data_pinter;
    struct ElementHeader *element_header = (struct ElementHeader *) ((char *) file_data_pinter + element_offset);

    if (element_header->has_prev_element_of_type) {
        struct ElementHeader *prev_element_of_type_header =
            (struct ElementHeader *) ((char *) file_data_pinter + element_header->prev_element_of_type_offset);
        prev_element_of_type_header->has_next_element_of_type = element_header->has_next_element_of_type;
        prev_element_of_type_header->next_element_of_type_offset = element_header->next_element_of_type_offset;
    }

    if (element_header->has_next_element_of_type) {
        struct ElementHeader *next_element_of_type_header =
            (struct ElementHeader *) ((char *) file_data_pinter + element_header->next_element_of_type_offset);
        next_element_of_type_header->has_prev_element_of_type = element_header->has_prev_element_of_type;
        next_element_of_type_header->prev_element_of_type_offset = element_header->prev_element_of_type_offset;
    }

    switch (element_header->element_type) {
        case ET_DELETED:
            if (file_header->last_deleted_element_offset == element_offset) {
                file_header->has_deleted_elements = element_header->has_prev_element_of_type;
                file_header->last_deleted_element_offset = element_header->prev_element_of_type_offset;
            }
            break;
        case ET_TABLE_METADATA:
            if (file_header->last_table_metadata_element_offset == element_offset) {
                file_header->has_table_metadata_elements = element_header->has_prev_element_of_type;
                file_header->last_table_metadata_element_offset = element_header->prev_element_of_type_offset;
            }
            break;
        case ET_TABLE_DATA:
            if (file_header->last_table_data_element_offset == element_offset) {
                file_header->has_table_data_elements = element_header->has_prev_element_of_type;
                file_header->last_table_data_element_offset = element_header->prev_element_of_type_offset;
            }
            break;
    }
}

void set_next_element_previous_to(const void *file_data_pointer, uint64_t previous, uint64_t set_to_offset) {

    struct ElementHeader *previous_header = (struct ElementHeader *) ((char *) file_data_pointer + previous);
    struct ElementHeader *next_element_header = (struct ElementHeader *) ((char *) file_data_pointer +
                                                                          previous +
                                                                          previous_header->element_size);
    next_element_header->prev_element_offset = set_to_offset;
}

void init_new_element_offsets_for_table_metadata(const uint64_t new_element_offset, void *file_data_pointer) {
    struct ElementHeader *allocated_element_header = (struct ElementHeader *) ((char *) file_data_pointer +
                                                                               new_element_offset);
    struct FileHeader *file_header = (struct FileHeader *) file_data_pointer;

    if (file_header->has_table_metadata_elements) {
        allocated_element_header->has_prev_element_of_type = true;
        allocated_element_header->prev_element_of_type_offset = file_header->last_table_metadata_element_offset;

        struct ElementHeader *last_table_metadata_element_header = (struct ElementHeader *) (
            (char *) file_data_pointer +
            file_header->last_table_metadata_element_offset);
        last_table_metadata_element_header->has_next_element_of_type = true;
        last_table_metadata_element_header->next_element_of_type_offset = new_element_offset;
    } else {
        allocated_element_header->has_prev_element_of_type = false;
    }

    file_header->has_table_metadata_elements = true;
    file_header->last_table_metadata_element_offset = new_element_offset;
    allocated_element_header->has_next_element_of_type = false;
}

void init_new_element_offsets_for_table_data(const uint64_t new_element_offset, void *file_data_pointer) {
    struct ElementHeader *allocated_element_header = (struct ElementHeader *) ((char *) file_data_pointer +
                                                                               new_element_offset);
    struct FileHeader *file_header = (struct FileHeader *) file_data_pointer;

    if (file_header->has_table_data_elements) {
        allocated_element_header->has_prev_element_of_type = true;
        allocated_element_header->prev_element_of_type_offset = file_header->last_table_data_element_offset;

        struct ElementHeader *last_table_data_element_header = (struct ElementHeader *) ((char *) file_data_pointer +
                                                                                         file_header->last_table_data_element_offset);
        last_table_data_element_header->has_next_element_of_type = true;
        last_table_data_element_header->next_element_of_type_offset = new_element_offset;
    } else {
        allocated_element_header->has_prev_element_of_type = false;
    }

    file_header->has_table_data_elements = true;
    file_header->last_table_data_element_offset = new_element_offset;
    allocated_element_header->has_next_element_of_type = false;
}

void init_new_element_offsets_for_deleted_element(const uint64_t new_element_offset, void *file_data_pointer) {
    struct ElementHeader *allocated_element_header = (struct ElementHeader *) ((char *) file_data_pointer +
                                                                               new_element_offset);
    struct FileHeader *file_header = (struct FileHeader *) file_data_pointer;

    if (file_header->has_deleted_elements) {
        allocated_element_header->has_prev_element_of_type = true;
        allocated_element_header->prev_element_of_type_offset = file_header->last_deleted_element_offset;

        struct ElementHeader *last_deleted_element_header = (struct ElementHeader *) ((char *) file_data_pointer +
                                                                                      file_header->last_deleted_element_offset);
        last_deleted_element_header->has_next_element_of_type = true;
        last_deleted_element_header->next_element_of_type_offset = new_element_offset;
    } else {
        allocated_element_header->has_prev_element_of_type = false;
    }

    file_header->has_deleted_elements = true;
    file_header->last_deleted_element_offset = new_element_offset;
    allocated_element_header->has_next_element_of_type = false;
}

void init_new_element_offsets(enum ElementType element_type, const uint64_t element_offset, void *file_data_pointer) {
    switch (element_type) {
        case ET_DELETED:
            init_new_element_offsets_for_deleted_element(element_offset, file_data_pointer);
            break;
        case ET_TABLE_METADATA:
            init_new_element_offsets_for_table_metadata(element_offset, file_data_pointer);
            break;
        case ET_TABLE_DATA:
            init_new_element_offsets_for_table_data(element_offset, file_data_pointer);
            break;
    }
}

void merge_deleted_with_next_if_can(uint64_t element_offset, void *file_data_pointer) {
    struct ElementHeader *element_header = (struct ElementHeader *) ((char *) file_data_pointer + element_offset);
    struct ElementHeader *next_element_header = (struct ElementHeader *) ((char *) file_data_pointer +
                                                                          element_offset +
                                                                          element_header->element_size);

    if (next_element_header->element_type == ET_DELETED) {
        logger(LL_DEBUG, __func__, "Next element is deleted. Merging.");
        element_header->element_size += next_element_header->element_size;
        erase_neighbors_data_about_element(file_data_pointer, element_offset + element_header->element_size);
    }
}

void prepare_deleted_element_for_allocation(void *file_data_pointer,
                                            uint64_t deleted_element_offset,
                                            uint64_t requested_element_size) {
    erase_neighbors_data_about_element(file_data_pointer, deleted_element_offset);

    struct ElementHeader *deleted_element_header = (struct ElementHeader *) ((char *) file_data_pointer +
                                                                             deleted_element_offset);
    struct FileHeader *file_header = (struct FileHeader *) file_data_pointer;

    uint64_t original_element_size = deleted_element_header->element_size;

    if (original_element_size < requested_element_size ||
        original_element_size - requested_element_size < MIN_ELEMENT_SIZE) {
        logger(LL_DEBUG, __func__, "Deleted element is too small to be splitted.");
        return;
    }

    logger(LL_DEBUG, __func__, "Splitting deleted element.");

    uint64_t new_element_offset = deleted_element_offset + requested_element_size;

    if (file_header->last_element_offset == deleted_element_offset) {
        file_header->last_element_offset = new_element_offset;
    }

    set_next_element_previous_to(file_data_pointer, deleted_element_offset, new_element_offset);

    uint64_t new_element_size = original_element_size - requested_element_size;
    struct ElementHeader *new_element_header = (struct ElementHeader *) ((char *) file_data_pointer +
                                                                         new_element_offset);
    new_element_header->element_size = new_element_size;
    new_element_header->element_type = ET_DELETED;
    new_element_header->prev_element_offset = deleted_element_offset;

    init_new_element_offsets(ET_DELETED, new_element_offset, file_data_pointer);

    deleted_element_header->element_size = requested_element_size;
}

int add_file_space_for_new_element(int fd, int mmap_result, uint64_t *file_size, void **file_data_pointer) {
    logger(LL_DEBUG, __func__, "File size is not enough to allocate new element.");
    int change_file_size_result = change_file_size(fd, (*file_size) + ALLOC_SIZE);
    if (change_file_size_result == -1) {
        return -1;
    }

    (*file_size) += ALLOC_SIZE;

    int munmap_result = munmap_file((*file_data_pointer), (*file_size), fd);
    if (munmap_result == -1) {
        return -1;
    }

    mmap_result = mmap_file(fd, file_data_pointer, 0, (*file_size));
    if (mmap_result == -1) {
        return -1;
    }

    return 0;
}

int allocate_element(int fd, uint64_t requested_element_size, enum ElementType element_type, uint64_t *element_offset) {
    logger(LL_DEBUG, __func__, "Allocating element of type %d with size %ld.", element_type, requested_element_size);

    requested_element_size += sizeof(struct ElementHeader);

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

    int find_suitable_deleted_element_offset_result =
        find_suitable_deleted_element_offset(file_data_pointer, requested_element_size,
                                             &suitable_deleted_element_offset);

    struct FileHeader *file_header = (struct FileHeader *) file_data_pointer;

    if (find_suitable_deleted_element_offset_result == 0) {
        logger(LL_DEBUG, __func__, "Found suitable deleted element of size %ld.", requested_element_size);
        prepare_deleted_element_for_allocation(file_data_pointer, suitable_deleted_element_offset,
                                               requested_element_size);

        *element_offset = suitable_deleted_element_offset;

        struct ElementHeader *allocated_element_header = (struct ElementHeader *) ((char *) file_data_pointer +
                                                                                   suitable_deleted_element_offset);
        allocated_element_header->element_type = element_type;

        init_new_element_offsets(element_type, *element_offset, file_data_pointer);

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
        int add_space_result = add_file_space_for_new_element(fd, mmap_result, &file_size, &file_data_pointer);
        if (add_space_result == -1) {
            return -1;
        }
    }

    *element_offset = file_header->last_element_offset + last_element_header->element_size;

    struct ElementHeader *allocated_element_header = (struct ElementHeader *) ((char *) file_data_pointer +
                                                                               *element_offset);

    allocated_element_header->element_size = requested_element_size;
    allocated_element_header->element_type = element_type;
    allocated_element_header->prev_element_offset = file_header->last_element_offset;
    file_header->last_element_offset += last_element_header->element_size;

    init_new_element_offsets(element_type, *element_offset, file_data_pointer);

    int munmap_result = munmap_file(file_data_pointer, file_size, fd);
    if (munmap_result == -1) {
        return -1;
    }

    return 0;
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

    if (element_header->element_type == ET_DELETED) {
        logger(LL_DEBUG, __func__, "Element is already deleted.");
        return -1;
    }

    // reset element's typed neighbors offsets to it
    erase_neighbors_data_about_element(file_data_pointer, element_offset);

    element_header->element_type = ET_DELETED;

    if (file_header->last_element_offset != element_offset) {
        merge_deleted_with_next_if_can(element_offset, file_data_pointer);
    }

    if (element_offset != FIRST_ELEMENT_OFFSET) {
        struct ElementHeader *prev_element_header = (struct ElementHeader *) ((char *) file_data_pointer +
                                                                              element_header->prev_element_offset);

        if (prev_element_header->element_type == ET_DELETED) {
            logger(LL_DEBUG, __func__, "Previous element is deleted. Merging.");
            prev_element_header->element_size += element_header->element_size;
            erase_neighbors_data_about_element(file_data_pointer, element_offset);
        } else {
            init_new_element_offsets(ET_DELETED, element_offset, file_data_pointer);
        }
    } else {
        init_new_element_offsets(ET_DELETED, element_offset, file_data_pointer);
    }

    return 0;
}
