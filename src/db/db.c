//
// Created by ruskaof on 19/10/23
//

#include "../utils/logging.h"
#include "db.h"

#include <stdint.h>

uint64_t file_size = 0;

bool has_first_page() {
    return file_size > 0;
}

#if defined(__linux__) || defined(__APPLE__)

#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>

int fd;

int init_db(const char *filename) {
    logger(LL_DEBUG, __func__, "Opening file %s.", filename);

    fd = open(filename, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);

    if (fd == -1) {
        logger(LL_ERROR, __func__, "Could not open file.");
        return -1;
    }

    file_size = (uint64_t) lseek(fd, 0, SEEK_END);

    return 0;
}

int close_db() {
    logger(LL_DEBUG, __func__, "Closing file with descriptor %d.", fd);

    int close_result = close(fd);

    if (close_result != 0) {
        logger(LL_ERROR, __func__, "Could not close file with descriptor %d.", fd);
        return -1;
    }

    return 0;
}

int change_file_size(uint64_t new_size) {
    logger(LL_DEBUG, __func__, "Changing file file size with descriptor %d to %ld.", fd, new_size);

    int result = ftruncate(fd, (off_t) new_size);

    if (result != 0) {
        logger(LL_ERROR, __func__, "Could not change file file size with descriptor %d to %ld.", fd,
               new_size);
        return -1;
    }

    file_size = new_size;
    return 0;
}

int sync_file() {
    logger(LL_DEBUG, __func__, "Syncing file with descriptor %d.", fd);

    int result = fsync(fd);

    if (result != 0) {
        logger(LL_ERROR, __func__, "Could not sync file with descriptor %d.", fd);
        return -1;
    }

    return 0;
}

struct PageHeader *mmap_page_header(uint64_t page_offset) {
    logger(LL_DEBUG, __func__, "Mapping page with offset %ld.", page_offset);

    struct PageHeader *page_header =mmap(NULL, PAGE_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, (off_t) page_offset);

    if (page_header == MAP_FAILED) {
        logger(LL_ERROR, __func__, "Could not map page with offset %ld.", page_offset);
        return NULL;
    }

    return page_header;
}

struct PageHeader *munmap_page_header(struct PageHeader *page_header) {
    logger(LL_DEBUG, __func__, "Unmapping page");

    int result = munmap(page_header, PAGE_SIZE);

    if (result != 0) {
        logger(LL_ERROR, __func__, "Could not unmap page.");
        return NULL;
    }

    return NULL;
}

int delete_db_file(const char *filename) {
    logger(LL_DEBUG, __func__, "Deleting file with name %s.", filename);

    int result = unlink(filename);

    if (result != 0) {
        logger(LL_ERROR, __func__, "Could not delete file with name %s.", filename);
        return -1;
    }

    return 0;
}

uint64_t allocate_page() {
    logger(LL_DEBUG, __func__, "Allocating page.");

    uint64_t new_page_offset = file_size;

    int result = change_file_size(file_size + PAGE_SIZE);

    if (result != 0) {
        logger(LL_ERROR, __func__, "Could not allocate page.");
        return -1;
    }

    return new_page_offset;
}

#endif

#if defined(_WIN32)


#endif