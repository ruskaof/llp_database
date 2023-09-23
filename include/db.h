//
// Created by Dmitriy Rusinov on 23.09.2023.
//

#ifndef LLP_DATABASE_DB_H
#define LLP_DATABASE_DB_H

#include "stddef.h"

struct TableField {
    char *name;
    void *data;
};

struct Row {
    size_t fields_length;
    struct TableField fields[];
};

struct SelectResult {
    size_t rows_length;
    struct Row rows[];
};

struct DbInfo {
    int file_descriptor;
    size_t file_size;
    void *file_data_pointer;
};

struct DbInfo init_db_file(const char *filename);

int close_db_file(struct DbInfo db_info);

struct SelectResult db_operation_select(const char *table_name);

#endif //LLP_DATABASE_DB_H
