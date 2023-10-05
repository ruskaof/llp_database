//
// Created by ruskaof on 2/10/23.
//

#include "allocator.h"
#include "file.h"

#include "../utils/logging.h"

void init_page_header(void *file_data_pointer,
                      uint64_t page_offset,
                      enum PageType page_type,
                      uint64_t page_size,
                      uint64_t prev_page_of_type_offset,
                      uint64_t next_page_of_type_offset) {
    struct PageHeader *page_header = (struct PageHeader *) (file_data_pointer + page_offset);

    page_header->page_size = page_size;
    page_header->has_elements = false;
    page_header->page_type = page_type;
    page_header->prev_page_of_type_offset = prev_page_of_type_offset;
    page_header->next_same_type_page_offset = next_page_of_type_offset;
}

void merge_deleted_page_with_next_and_prev_pages(void *file_data_pointer,
                                                 uint64_t file_size
) {
    if (!page->is_deleted) {
        logger(LL_ERROR, __func__, "Page is not deleted.");
        return;
    }

    struct PageHeader *next_page = (struct PageHeader *) ((char *) page + page->page_size);

    while (next_page->is_deleted && (char *) next_page < (char *) file_data_pointer + file_size) {
        logger(LL_DEBUG, __func__, "Merging page of size %ld with next page of size %ld.", page->page_size,
               next_page->page_size);
        page->page_size += next_page->page_size;
        next_page = (struct PageHeader *) ((char *) page + page->page_size);
    }
}

int allocate_page_in_empty_file(int fd,
                                uint64_t min_size,
                                enum PageType page_type,
                                uint64_t *page_offset,
                                uint64_t *page_size) {
    logger(LL_DEBUG, __func__, "File is empty, allocating first page.");
    int change_file_size_result = change_file_size(fd, min_size + FIRST_PAGE_OFFSET);
    if (change_file_size_result == -1) {
        return -1;
    }

    *page_offset = FIRST_PAGE_OFFSET;
    *page_size = min_size;

    void *file_data_pointer;
    int mmap_result = mmap_file(fd, &file_data_pointer, 0, min_size);
    if (mmap_result == -1) {
        return -1;
    }

    struct FileHeader *file_header = (struct FileHeader *) file_data_pointer;
    file_header->first_deleted_page_offset = -1;
    file_header->has_deleted_pages = false;

    init_page_header(file_data_pointer,
                     FIRST_PAGE_OFFSET,
                     page_type,
                     min_size,
                     0,
                     0);

    sync_file(fd);

    int munmap_result = munmap_file(file_data_pointer, min_size);
    if (munmap_result == -1) {
        return -1;
    }

    logger(LL_DEBUG, __func__, "Allocated first page of size %ld in offset %ld.", min_size, 0);
}

int find_suitable_deleted_page(void *file_data_pointer,
                               uint64_t file_size,
                               uint64_t min_size,
                               uint64_t *suitable_deleted_page_offset) {
    logger(LL_DEBUG, __func__, "Finding suitable deleted page of size %ld.", min_size);

    struct FileHeader *file_header = (struct FileHeader *) file_data_pointer;
    if (file_header->has_deleted_pages == false) {
        return -1;
    }

    uint64_t current_offset = file_header->first_deleted_page_offset;
    while (current_offset < file_size) {
        struct PageHeader *page_header = (struct PageHeader *) ((char *) file_data_pointer + current_offset);
        if (page_header->page_size >= min_size) {
            *suitable_deleted_page_offset = current_offset;
            return 0;
        }
        struct DeletedPageSubHeader *deleted_page_sub_header =
            (struct DeletedPageSubHeader *) ((char *) page_header + sizeof(struct PageHeader));

        if (deleted_page_sub_header->is_last_deleted_page) {
            return -1;
        }

        current_offset = deleted_page_sub_header->next_deleted_page_offset;
    }

    return -1;
}

void set_offsets_to_deleted_page(const void *file_data_pointer, uint64_t prev_deleted_page_offset,
                                 uint64_t next_deleted_page_offset, uint64_t split_page_offset) {
    if (prev_deleted_page_offset == 0) {
        struct FileHeader *file_header = (struct FileHeader *) file_data_pointer;
        file_header->first_deleted_page_offset = split_page_offset;
    } else {
        struct PageHeader *prev_deleted_page_header = (struct PageHeader *) ((char *) file_data_pointer +
                                                                             prev_deleted_page_offset);
        prev_deleted_page_header->next_page_of_type_offset = split_page_offset;

        struct PageHeader *next_deleted_page_header = (struct PageHeader *) ((char *) file_data_pointer +
                                                                             next_deleted_page_offset);
        next_deleted_page_header->prev_page_of_type_offset = split_page_offset;
    }
}

int split_deleted_page_for_allocation(
    void *file_data_pointer,
    uint64_t page_offset,
    uint64_t space_for_page
) {
    struct PageHeader *page_header = (struct PageHeader *) ((char *) file_data_pointer + page_offset);

    uint64_t original_page_size = page_header->page_size;
    if (original_page_size - space_for_page >= MIN_PAGE_SIZE) {
        logger(LL_DEBUG, __func__, "Page is big enough to be splitted, splitting it.");

        uint64_t prev_deleted_page_offset = page_header->prev_page_of_type_offset;
        uint64_t next_deleted_page_offset = page_header->next_page_of_type_offset;

        page_header->page_size = space_for_page;

        struct PageHeader *new_page_header = (struct PageHeader *) ((char *) page_header + space_for_page);
        new_page_header->page_size = original_page_size - space_for_page;

        uint64_t split_page_offset = page_offset + space_for_page;

        init_page_header(file_data_pointer,
                         split_page_offset,
                         PT_DELETED_PAGE,
                         original_page_size - space_for_page,
                         prev_deleted_page_offset,
                         next_deleted_page_offset);

        set_offsets_to_deleted_page(file_data_pointer, prev_deleted_page_offset, next_deleted_page_offset,
                                    split_page_offset);
    } else {
        logger(LL_DEBUG, __func__, "Page is not big enough to be splitted, allocating it.");
    }
}

int allocate_page(int fd, uint64_t min_size, enum PageType page_type, uint64_t *page_offset, uint64_t *page_size) {
    logger(LL_DEBUG, __func__, "Allocating page of type %d with min size %ld.", page_type, min_size);

    uint64_t file_size = get_file_size(fd);
    if (file_size == -1) {
        return -1;
    }

    if (min_size < MIN_PAGE_SIZE) {
        min_size = MIN_PAGE_SIZE;
    }

    if (file_size == 0) {
        return allocate_page_in_empty_file(fd, min_size, page_type, page_offset, page_size);
    }

    void *file_data_pointer;

    int mmap_result = mmap_file(fd, &file_data_pointer, 0, file_size);
    if (mmap_result == -1) {
        return -1;
    }

    uint64_t *suitable_deleted_page_offset;
    int find_suitable_deleted_page_result = find_suitable_deleted_page(file_data_pointer, file_size, min_size,
                                                                       suitable_deleted_page_offset);

    if (find_suitable_deleted_page_result == 0) {
        logger(LL_DEBUG, __func__, "Found suitable deleted page of size %ld.", min_size);
        split_deleted_page_for_allocation(file_data_pointer, *suitable_deleted_page_offset, min_size);

        *page_offset = *suitable_deleted_page_offset;
        *page_size = min_size;
        int munmap_result = munmap_file(file_data_pointer, file_size);
        if (munmap_result == -1) {
            return -1;
        }
        return 0;
    }

    uint64_t current_offset = 0;

    while (current_offset < file_size) {
        struct PageHeader *page_header = (struct PageHeader *) ((char *) file_data_pointer + current_offset);

        if (page_header->is_deleted && page_header->page_size <= min_size) {

        }
    }

    logger(LL_DEBUG, __func__, "Found deleted page of size %ld.", page_header->page_size);

    uint64_t original_page_size = page_header->page_size;
    if (original_page_size - min_size >= MIN_PAGE_SIZE) {
        logger(LL_DEBUG, __func__, "Page is big enough to be splitted, splitting it.");
        page_header->page_size = min_size;
        struct PageHeader *new_page_header = (struct PageHeader *) ((char *) page_header + min_size);
        init_page_header(new_page_header, page_type, original_page_size - min_size, true);
    } else {
        logger(LL_DEBUG, __func__, "Page is not big enough to be splitted, allocating it.");
        page_header->page_size = original_page_size;
    }

    *page_offset = (char *) page_header - (char *) file_data_pointer;
    *page_size = page_header->page_size;
    init_page_header((struct PageHeader *) (page_header), page_type, page_header->page_size, false);

    sync_file(fd);

    int munmap_result = munmap_file(file_data_pointer, file_size);
    if (munmap_result == -1) {
        return -1;
    }

    return 0;
}

int delete_page(int fd, uint64_t page_offset) {
    logger(LL_DEBUG, __func__, "Deleting page at offset %ld.", page_offset);

    uint64_t file_size = get_file_size(fd);
    if (file_size == -1) {
        return -1;
    }

    void *file_data_pointer;

    int mmap_result = mmap_file(fd, &file_data_pointer, 0, file_size);
    if (mmap_result == -1) {
        return -1;
    }

    struct PageHeader *page_header = (struct PageHeader *) ((char *) file_data_pointer + page_offset);
    page_header->is_deleted = true;

    sync_file(fd);

    int munmap_result = munmap_file(file_data_pointer, file_size);
    if (munmap_result == -1) {
        return -1;
    }

    return 0;
}
