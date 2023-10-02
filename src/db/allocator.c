//
// Created by ruskaof on 2/10/23.
//

#include "allocator.h"
#include "file.h"

#include "../utils/logging.h"

void init_page_header(struct PageHeader *page_header, enum PageType page_type, uint64_t page_size) {
    page_header->is_deleted = false;
    page_header->page_size = page_size;
    page_header->has_elements = false;
    page_header->page_type = page_type;
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
        init_page_header((struct PageHeader *) file_data_pointer, page_type, min_size);

        sync_file(fd);

        int munmap_result = munmap_file(file_data_pointer, min_size);
        if (munmap_result == -1) {
            return -1;
        }

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

            init_page_header((struct PageHeader *) ((char *) file_data_pointer + file_size), page_type, min_size);

            sync_file(fd);

            munmap_result = munmap_file(file_data_pointer, file_size);
            if (munmap_result == -1) {
                return -1;
            }

            return 0;
        } else {
            page_header = next_page_header;
        }
    }

    logger(LL_DEBUG, __func__, "Found deleted page of size %ld.", page_header->page_size);

    *page_offset = (char *) page_header - (char *) file_data_pointer;
    *page_size = page_header->page_size;
    init_page_header((struct PageHeader *) (page_header), page_type, page_header->page_size);

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
