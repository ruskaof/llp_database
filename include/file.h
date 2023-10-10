//
// Created by ruskaof on 10/10/23.
//

#ifndef LLP_DATABASE_FILE_H
#define LLP_DATABASE_FILE_H

#include <stdint.h>

int open_file(const char *filename);

int close_file();

uint64_t get_file_size();

#endif //LLP_DATABASE_FILE_H