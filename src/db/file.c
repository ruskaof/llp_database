//
// Created by ruskaof on 1/10/23.
//

#include "file.h"
#include "../utils/logging.h"

#include <unistd.h>
#include <fcntl.h>

void *file_data_pointer;
uint64_t file_size = 0;

uint64_t get_file_size() {
    return file_size;
}

void *get_file_data_pointer() {
    return file_data_pointer;
}

#if defined(__linux__) || defined(__APPLE__)

#include <sys/mman.h>

int fd;

int init_db(const char *filename) {
    logger(LL_DEBUG, __func__, "Opening file %s.", filename);

    fd = open(filename, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);

    if (fd == -1) {
        logger(LL_ERROR, __func__, "Could not open file.");
        return -1;
    }

    file_size = (uint64_t) lseek(fd, 0, SEEK_END);

    if (file_size != 0) {
        int mmap_result = mmap_file();
        if (mmap_result == -1) {
            logger(LL_ERROR, __func__, "Could not mmap file.");
            return -1;
        }
    }

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

int munmap_file() {
    if (file_size == 0) {
        return 0;
    }

    logger(LL_DEBUG, __func__, "Unmapping file with pointer %p and file size %ld.",
           file_data_pointer, file_size);

    int sync_result = sync_file();
    if (sync_result == -1) {
        logger(LL_ERROR, __func__, "Could not sync file with pointer %p and file size %ld.",
               file_data_pointer, file_size);
        return -1;
    }

    int result = munmap(file_data_pointer, file_size);

    if (result != 0) {
        logger(LL_ERROR, __func__, "Could not unmap file with pointer %p and file size %ld.",
               file_data_pointer, file_size);
        return -1;
    }

    return 0;
}

int mmap_file() {
    logger(LL_DEBUG, __func__, "Mapping file with descriptor %d and file size %ld.", fd, file_size);

    if (file_size == 0) {
        return 0;
    }

    file_data_pointer = mmap(NULL, file_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, (off_t) 0);

    if (file_data_pointer == MAP_FAILED) {
        logger(LL_ERROR, __func__, "Could not map file with descriptor %d and file size %ld.", fd, file_size);
        return -1;
    }

    return 0;
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

#endif

#if defined(_WIN32)

#include <windows.h>

HANDLE hFile;

int open_file(const char *filename) {
    logger(LL_DEBUG, __func__, "Opening file %s.", filename);

    hFile = CreateFile(filename, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

    if (hFile == INVALID_HANDLE_VALUE) {
        logger(LL_ERROR, __func__, "Could not open file.");
        return -1;
    }

    file_size = (uint64_t) GetFileSize(hFile, NULL);

    return 0;
}

int close_file() {
    logger(LL_DEBUG, __func__, "Closing file with handle %p.", hFile);

    int close_result = CloseHandle(hFile);

    if (close_result == 0) {
        logger(LL_ERROR, __func__, "Could not close file with handle %p.", hFile);
        return -1;
    }

    return 0;
}

int change_file_size(uint64_t new_size) {
    logger(LL_DEBUG, __func__, "Changing file file size with handle %p to %ld.", hFile, new_size);

    int result = SetFilePointer(hFile, (LONG) new_size, NULL, FILE_BEGIN);
    if (result == INVALID_SET_FILE_POINTER) {
        logger(LL_ERROR, __func__, "Could not change file file size with handle %p to %ld.", hFile,
               new_size);
        return -1;
    }

    result = SetEndOfFile(hFile);
    if (result == 0) {
        logger(LL_ERROR, __func__, "Could not change file file size with handle %p to %ld.", hFile,
               new_size);
        return -1;
    }

    file_size = new_size;
    return 0;
}

int sync_file() {
    logger(LL_DEBUG, __func__, "Syncing file with handle %p.", hFile);

    int result = FlushFileBuffers(hFile);

    if (result == 0) {
        logger(LL_ERROR, __func__, "Could not sync file with handle %p.", hFile);
        return -1;
    }

    return 0;
}

int munmap_file() {
    if (file_size == 0) {
        return 0;
    }

    logger(LL_DEBUG, __func__, "Unmapping file with pointer %p and file size %ld.",
           file_data_pointer, file_size);

    int sync_result = sync_file();
    if (sync_result == -1) {
        logger(LL_ERROR, __func__, "Could not sync file with pointer %p and file size %ld.",
               file_data_pointer, file_size);
        return -1;
    }

    int result = UnmapViewOfFile(file_data_pointer);

    if (result == 0) {
        logger(LL_ERROR, __func__, "Could not unmap file with pointer %p and file size %ld.",
               file_data_pointer, file_size);
        return -1;
    }

    return 0;
}

int mmap_file() {
    logger(LL_DEBUG, __func__, "Mapping file with handle %p and file size %ld.", hFile, file_size);

    if (file_size == 0) {
        return 0;
    }

    HANDLE hMapFile = CreateFileMapping(hFile, NULL, PAGE_READWRITE, 0, 0, NULL);
    if (hMapFile == NULL) {
        logger(LL_ERROR, __func__, "Could not create file mapping for file with handle %p and file size %ld.", hFile,
               file_size);
        return -1;
    }

    file_data_pointer = MapViewOfFile(hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, 0);
    if (file_data_pointer == NULL) {
        logger(LL_ERROR, __func__, "Could not map file with handle %p and file size %ld.", hFile, file_size);
        return -1;
    }

    return 0;
}

int delete_file(const char *filename) {
    logger(LL_DEBUG, __func__, "Deleting file with name %s.", filename);

    int result = DeleteFile(filename);

    if (result == 0) {
        logger(LL_ERROR, __func__, "Could not delete file with name %s.", filename);
        return -1;
    }

    return 0;
}

#endif
