//
// Created by ruskaof on 1/10/23.
//

#ifndef LLP_DATABASE_FILE_H
#define LLP_DATABASE_FILE_H

#include <stdlib.h>

int open_file(const char *filename);

int close_file(int file_descriptor);

int change_file_size(int fd, long new_size);

int sync_file(int fd);

int munmap_file(void *file_data_pointer, size_t file_size);

int mmap_file(int fd, size_t file_size, void **file_data_pointer);

int delete_file(const char *filename);

size_t get_file_size(int fd);

#endif //LLP_DATABASE_FILE_H
