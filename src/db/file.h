//
// Created by ruskaof on 1/10/23.
//

#ifndef LLP_DATABASE_FILE_H
#define LLP_DATABASE_FILE_H

#include "../../include/db.h"

#include <stdlib.h>
#include <stdint.h>

void *get_file_data_pointer();

uint64_t get_file_size();

int change_file_size(uint64_t new_size);

int munmap_file();

int mmap_file();

#endif //LLP_DATABASE_FILE_H
