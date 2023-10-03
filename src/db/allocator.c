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
                      uint64_t prev_page_offset) {
    struct PageHeader *page_header = (struct PageHeader *) file_data_pointer;

    page_header->page_size = page_size;
    page_header->has_elements = false;
    page_header->page_type = page_type;
    page_header->prev_page_offset = prev_page_offset;
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
    int change_file_size_result = change_file_size(fd, min_size);
    if (change_file_size_result == -1) {
        return -1;
    }

    *page_offset = 0;
    *page_size = min_size;

    void *file_data_pointer;
    int mmap_result = mmap_file(fd, &file_data_pointer, 0, min_size);
    if (mmap_result == -1) {
        return -1;
    }
    init_page_header(file_data_pointer, 0, page_type, min_size, false, -1);

    sync_file(fd);

    int munmap_result = munmap_file(file_data_pointer, min_size);
    if (munmap_result == -1) {
        return -1;
    }

    logger(LL_DEBUG, __func__, "Allocated first page of size %ld in offset %ld.", min_size, 0);
}

int split_page_for_allocation(
    void *file_data_pointer,
    uint64_t page_offset,
    uint64_t min_size
) {
    struct PageHeader *page_header = (struct PageHeader *) ((char *) file_data_pointer + page_offset);

    uint64_t original_page_size = page_header->page_size;
    if (original_page_size - min_size >= MIN_PAGE_SIZE) {
        logger(LL_DEBUG, __func__, "Page is big enough to be splitted, splitting it.");
        page_header->page_size = min_size;
        init_page_header(file_data_pointer, page_offset + min_size, PT_DELETED_PAGE, original_page_size - min_size,
                         false,
                         page_offset);
    } else {
        logger(LL_DEBUG, __func__, "Page is not big enough to be splitted, allocating it.");
        page_header->page_size = original_page_size;
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
