//
// Created by ruskaof on 23/09/23.
//

#ifndef LLP_DATABASE_TABLE_OPERATIONS_H
#define LLP_DATABASE_TABLE_OPERATIONS_H

#include "table_schema.h"
#include "../src/db/file/file_internal.h"

#include <stdlib.h>

struct SelectRequest {
    char *table_name;
    size_t max_rows;
    size_t requested_fields_length;
    char *requested_fields[];
};

struct TableFieldData {
    struct TableFieldData *next_data;
    enum TableFieldDataType data_type;
    size_t data_size;
    void *data;
};

struct TableFieldData *operation_select(
    char *table_name,
    size_t max_rows,
    size_t requested_fields_length,
    char *requested_fields[],
    struct FileDbInfo fileDbInfo
);

int operation_insert(
    char *table_name,
    struct TableFieldData *field_data
);

#endif //LLP_DATABASE_TABLE_OPERATIONS_H
