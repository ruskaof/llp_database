//
// Created by ruskaof on 1/10/23.
//

#include "file.h"
#include "../utils/logging.h"

#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <stdint.h>

int open_file(const char *filename) {
    int fd = open(filename, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);

    if (fd == -1) {
        logger(LL_ERROR, __func__, "Could not open file.");
        return -1;
    }

    logger(LL_DEBUG, __func__, "Opened file with descriptor %d.", fd);
    return fd;
}

int close_file(int file_descriptor) {
    int close_result = close(file_descriptor);

    if (close_result != 0) {
        logger(LL_ERROR, __func__, "Could not close file with descriptor %d.", file_descriptor);
        return -1;
    }

    logger(LL_DEBUG, __func__, "Closed file with descriptor %d.", file_descriptor);
    return 0;
}

int change_file_size(int fd, long new_size) {
    int result = ftruncate(fd, new_size);

    if (result != 0) {
        logger(LL_ERROR, __func__, "Could not change file buckets_count with descriptor %d to %ld.", fd,
               new_size);
        return -1;
    }

    logger(LL_DEBUG, __func__, "Changed file buckets_count with descriptor %d to %ld.", fd, new_size);
    return 0;
}

int sync_file(int fd) {
    int result = fsync(fd);

    if (result != 0) {
        logger(LL_ERROR, __func__, "Could not sync file with descriptor %d.", fd);
        return -1;
    }

    logger(LL_DEBUG, __func__, "Synced file with descriptor %d.", fd);
    return 0;
}

int munmap_file(void *file_data_pointer, size_t file_size) {
    int result = munmap(file_data_pointer, file_size);

    if (result != 0) {
        logger(LL_ERROR, __func__, "Could not unmap file with pointer %p and buckets_count %ld.",
               file_data_pointer, file_size);
        return -1;
    }

    logger(LL_DEBUG, __func__, "Unmapped file with pointer %p and buckets_count %ld.", file_data_pointer,
           file_size);
    return 0;
}

int mmap_file(int fd, void **file_data_pointer, off_t offset, size_t size) {
    *file_data_pointer = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, offset);

    if (*file_data_pointer == MAP_FAILED) {
        logger(LL_ERROR, __func__, "Could not map file with descriptor %d, offset %ld and buckets_count %ld.", fd,
               offset, size);
        return -1;
    }

    logger(LL_DEBUG, __func__, "Mapped file with descriptor %d, offset %ld and buckets_count %ld.", fd, offset,
           size);
    return 0;
}

int delete_file(const char *filename) {
    int result = unlink(filename);

    if (result != 0) {
        logger(LL_ERROR, __func__, "Could not delete file with name %s.", filename);
        return -1;
    }

    logger(LL_DEBUG, __func__, "Deleted file with name %s.", filename);
    return 0;
}

size_t get_file_size(int fd) {
    size_t file_size = lseek(fd, 0, SEEK_END);

    if (file_size == -1) {
        logger(LL_ERROR, __func__, "Could not get file size with descriptor %d.", fd);
        return -1;
    }

    logger(LL_DEBUG, __func__, "Got file size with descriptor %d.", fd);
    return (size_t) file_size;
}
