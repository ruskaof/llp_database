//
// Created by ruskaof on 2/10/23.
//

#include "allocator.h"
#include "file.h"

#include "../utils/logging.h"

void init_page_header(struct PageHeader *page_header, enum PageType page_type, uint64_t page_size, bool is_deleted) {
    page_header->is_deleted = is_deleted;
    page_header->page_size = page_size;
    page_header->has_elements = false;
    page_header->page_type = page_type;
}

void merge_deleted_page_with_next_pages(struct PageHeader *page, uint64_t file_size, void *file_data_pointer) {
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
        init_page_header((struct PageHeader *) file_data_pointer, page_type, min_size, false);

        sync_file(fd);

        int munmap_result = munmap_file(file_data_pointer, min_size);
        if (munmap_result == -1) {
            return -1;
        }

        logger(LL_DEBUG, __func__, "Allocated first page of size %ld in offset %ld.", min_size, 0);

        return 0;
    }

    void *file_data_pointer;

    int mmap_result = mmap_file(fd, &file_data_pointer, 0, file_size);
    if (mmap_result == -1) {
        return -1;
    }

    struct PageHeader *page_header = (struct PageHeader *) file_data_pointer;

    while (!page_header->is_deleted || page_header->page_size < min_size) {
        struct PageHeader *next_page_header = (struct PageHeader *) ((char *) page_header + page_header->page_size);

        if (next_page_header->is_deleted) {
            logger(LL_DEBUG, __func__, "Found deleted page of size %ld. Merging it with next ones",
                   next_page_header->page_size);
            merge_deleted_page_with_next_pages(next_page_header, file_size, file_data_pointer);
            if (next_page_header->page_size >= min_size) {
                logger(LL_DEBUG, __func__, "Merged page is big enough, allocating it.");
                page_header = next_page_header;
                break;
            }
        }

        if ((char *) next_page_header >= (char *) file_data_pointer + file_size) {
            logger(LL_DEBUG, __func__, "No deleted page found, allocating new page.");
            int change_file_size_result = change_file_size(fd, file_size + min_size);
            if (change_file_size_result == -1) {
                return -1;
            }

            int munmap_result = munmap_file(file_data_pointer, file_size);
            if (munmap_result == -1) {
                return -1;
            }

            mmap_result = mmap_file(fd, &file_data_pointer, 0, file_size + min_size);
            if (mmap_result == -1) {
                return -1;
            }

            *page_offset = file_size;
            *page_size = min_size;

            init_page_header((struct PageHeader *) ((char *) file_data_pointer + file_size), page_type, min_size,
                             false);

            sync_file(fd);

            munmap_result = munmap_file(file_data_pointer, file_size);
            if (munmap_result == -1) {
                return -1;
            }

            logger(LL_DEBUG, __func__, "Allocated first page of size %ld in offset %ld.", min_size, file_size);

            return 0;
        } else {
            page_header = next_page_header;
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
