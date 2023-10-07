//
// Created by ruskaof on 1/10/23.
//

#include "file.h"
#include "../utils/logging.h"

#include <unistd.h>
#include <fcntl.h>

#if defined(__linux__) || defined(__APPLE__)

#include <sys/mman.h>

int open_file(const char *filename) {
    int fd = open(filename, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);

    if (fd == -1) {
        logger(LL_ERROR, __func__, "Could not open file.");
        return -1;
    }

    return fd;
}

int close_file(int file_descriptor) {
    int close_result = close(file_descriptor);

    if (close_result != 0) {
        logger(LL_ERROR, __func__, "Could not close file with descriptor %d.", file_descriptor);
        return -1;
    }

    return 0;
}

int change_file_size(int fd, uint64_t new_size) {
    int result = ftruncate(fd, (off_t) new_size);

    if (result != 0) {
        logger(LL_ERROR, __func__, "Could not change file file size with descriptor %d to %ld.", fd,
               new_size);
        return -1;
    }

    return 0;
}

int sync_file(int fd) {
    int result = fsync(fd);

    if (result != 0) {
        logger(LL_ERROR, __func__, "Could not sync file with descriptor %d.", fd);
        return -1;
    }

    return 0;
}

int munmap_file(void *file_data_pointer, uint64_t file_size, int fd) {
    int result = munmap(file_data_pointer, file_size);

    if (result != 0) {
        logger(LL_ERROR, __func__, "Could not unmap file with pointer %p and file size %ld.",
               file_data_pointer, file_size);
        return -1;
    }

    sync_file(fd);

    return 0;
}

int mmap_file(int fd, void **file_data_pointer, uint64_t offset, uint64_t size) {
    *file_data_pointer = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, (off_t) offset);

    if (*file_data_pointer == MAP_FAILED) {
        logger(LL_ERROR, __func__, "Could not map file with descriptor %d, offset %ld and file size %ld.", fd,
               offset, size);
        return -1;
    }

    return 0;
}

int delete_file(const char *filename) {
    int result = unlink(filename);

    if (result != 0) {
        logger(LL_ERROR, __func__, "Could not delete file with name %s.", filename);
        return -1;
    }

    return 0;
}

uint64_t get_file_size(int fd) {
    off_t file_size = lseek(fd, 0, SEEK_END);

    if (file_size == -1) {
        logger(LL_ERROR, __func__, "Could not get file size with descriptor %d.", fd);
        return -1;
    }

    return file_size;
}

#endif

#if defined(_WIN32)

#include <windows.h>

int open_file(const char *filename) {
    HANDLE fd = CreateFile(filename, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

    if (fd == INVALID_HANDLE_VALUE) {
        logger(LL_ERROR, __func__, "Could not open file.");
        return -1;
    }

    return (int) fd;
}

int close_file(int file_descriptor) {
    int close_result = CloseHandle((HANDLE) file_descriptor);

    if (close_result == 0) {
        logger(LL_ERROR, __func__, "Could not close file with descriptor %d.", file_descriptor);
        return -1;
    }

    return 0;
}

int change_file_size(int fd, uint64_t new_size) {
    LARGE_INTEGER file_size;
    file_size.QuadPart = new_size;

    int result = SetFilePointerEx((HANDLE) fd, file_size, NULL, FILE_BEGIN);

    if (result == 0) {
        logger(LL_ERROR, __func__, "Could not change file file size with descriptor %d to %ld.", fd,
               new_size);
        return -1;
    }

    result = SetEndOfFile((HANDLE) fd);

    if (result == 0) {
        logger(LL_ERROR, __func__, "Could not change file file size with descriptor %d to %ld.", fd,
               new_size);
        return -1;
    }

    return 0;
}

int sync_file(int fd) {
    int result = FlushFileBuffers((HANDLE) fd);

    if (result == 0) {
        logger(LL_ERROR, __func__, "Could not sync file with descriptor %d.", fd);
        return -1;
    }

    return 0;
}

int munmap_file(void *file_data_pointer, uint64_t file_size, int fd) {
    int result = UnmapViewOfFile(file_data_pointer);

    if (result == 0) {
        logger(LL_ERROR, __func__, "Could not unmap file with pointer %p and file size %ld.",
               file_data_pointer, file_size);
        return -1;
    }

    sync_file(fd);

    return 0;
}

int mmap_file(int fd, void **file_data_pointer, uint64_t offset, uint64_t size) {
    HANDLE file_mapping = CreateFileMapping((HANDLE) fd, NULL, PAGE_READWRITE, 0, 0, NULL);

    if (file_mapping == NULL) {
        logger(LL_ERROR, __func__, "Could not create file mapping with descriptor %d, offset %ld and file size %ld.",
               fd, offset, size);
        return -1;
    }

    *file_data_pointer = MapViewOfFile(file_mapping, FILE_MAP_ALL_ACCESS, 0, 0, size);

    if (*file_data_pointer == NULL) {
        logger(LL_ERROR, __func__, "Could not map file with descriptor %d, offset %ld and file size %ld.", fd,
               offset, size);
        return -1;
    }

    return 0;
}

int delete_file(const char *filename) {
    int result = DeleteFile(filename);

    if (result == 0) {
        logger(LL_ERROR, __func__, "Could not delete file with name %s.", filename);
        return -1;
    }

    return 0;
}

uint64_t get_file_size(int fd) {
    LARGE_INTEGER file_size;

    int result = GetFileSizeEx((HANDLE) fd, &file_size);

    if (result == 0) {
        logger(LL_ERROR, __func__, "Could not get file size with descriptor %d.", fd);
        return -1;
    }

    return (uint64_t) file_size.QuadPart;
}

#endif
