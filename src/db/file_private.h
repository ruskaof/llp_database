//
// Created by ruskaof on 1/10/23.
//

#ifndef LLP_DATABASE_FILE_PRIVATE_H
#define LLP_DATABASE_FILE_PRIVATE_H

#include "../../include/file.h"

#include <stdlib.h>
#include <stdint.h>

int change_file_size(uint64_t new_size);

int sync_file();

int munmap_file(void *file_data_pointer, uint64_t file_size);

int mmap_file(void **file_data_pointer, uint64_t offset, uint64_t size);

int delete_file(const char *filename);

#endif //LLP_DATABASE_FILE_PRIVATE_H
