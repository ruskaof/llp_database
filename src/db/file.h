//
// Created by ruskaof on 1/10/23.
//

#ifndef LLP_DATABASE_FILE_H
#define LLP_DATABASE_FILE_H

#include <stdlib.h>
#include <stdint.h>

int open_file(const char *filename);

int close_file(int file_descriptor);

int change_file_size(int fd, uint64_t new_size);

int sync_file(int fd);

int munmap_file(void *file_data_pointer, uint64_t file_size);

int mmap_file(int fd, void **file_data_pointer, uint64_t offset, uint64_t size);

int delete_file(const char *filename);

uint64_t get_file_size(int fd);

#endif //LLP_DATABASE_FILE_H
