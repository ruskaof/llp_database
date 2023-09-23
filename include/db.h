//
// Created by Dmitriy Rusinov on 23.09.2023.
//

#ifndef LLP_DATABASE_DB_H
#define LLP_DATABASE_DB_H

#include "stddef.h"

struct Field {
    char *name;
    void *data;
};

struct Row {
    size_t fields_length;
    struct Field fields[];
};

struct SelectResult {
    size_t rows_length;
    struct Row rows[];
};

struct DbInfo {
    int fd;
    void *file_data_pointer;
};

struct DbInfo init_db_file(const char *filename);
void close_file(int file_descriptor);

struct SelectResult db_operation_select(const char *table_name);

#endif //LLP_DATABASE_DB_H
