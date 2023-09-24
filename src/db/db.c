//
// Created by ruskaof on 24/09/23.
//

#include "../../include/db.h"
#include "db.h"
#include "file/file_internal.h"
#include "../utils/logging.h"
#include "hash_table/hash_table.h"

struct OpenedDbInfo init_db_in_file(const char *filename) {
    int fd = open_file(filename);
    if (fd == -1) {
        logger(LL_ERROR, __func__, "Failed to open file %s.", filename);
        return (struct OpenedDbInfo) {-1, 0, NULL};
    }

    size_t file_size = get_file_size(fd);

    if (file_size < DB_FILE_HEADER_SIZE) {
        logger(LL_DEBUG, __func__, "File buckets_count is %ld, which is less than the header buckets_count of %d.",
               file_size, DB_FILE_HEADER_SIZE);
        change_file_size(fd, DB_FILE_HEADER_SIZE);
    }

    void *file_data_pointer = NULL;
    int mmap_result = mmap_file(fd, DB_FILE_HEADER_SIZE, &file_data_pointer);
    if (mmap_result == -1) {
        logger(LL_ERROR, __func__, "Failed to mmap file %s.", filename);
        return (struct OpenedDbInfo) {-1, 0, NULL};
    }

    struct HashTable *hash_table = (struct HashTable *) file_data_pointer;
    hash_table->buckets_count = TABLE_HASH_SIZE;
    create_hash_table(hash_table->buckets_count, hash_table);

    int sync_file_result = sync_file(fd);
    if (sync_file_result == -1) {
        logger(LL_ERROR, __func__, "Failed to sync file %s.", filename);
        return (struct OpenedDbInfo) {-1, 0, NULL};
    }

    int munmap_file_result = munmap_file(file_data_pointer, DB_FILE_HEADER_SIZE);
    if (munmap_file_result == -1) {
        logger(LL_ERROR, __func__, "Failed to munmap file %s.", filename);
        return (struct OpenedDbInfo) {-1, 0, NULL};
    }

    return (struct OpenedDbInfo) {fd, file_size, NULL};
}

int close_db(struct OpenedDbInfo opened_db_info) {
    int munmap_result = munmap_file(opened_db_info.mmaped_file, opened_db_info.file_size);
    if (munmap_result == -1) {
        logger(LL_ERROR, __func__, "Failed to munmap file.");
        return -1;
    }

    int close_result = close_file(opened_db_info.fd);
    if (close_result == -1) {
        logger(LL_ERROR, __func__, "Failed to close file.");
        return -1;
    }

    return 0;
}
