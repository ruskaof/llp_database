//
// Created by ruskaof on 23/09/23.
//

#ifndef LLP_DATABASE_TABLE_SCHEMA_H
#define LLP_DATABASE_TABLE_SCHEMA_H

#include <stdlib.h>

enum TableFieldDataType {
    FIELD_DATA_TYPE_INT32,
    FIELD_DATA_TYPE_INT64,
    FIELD_DATA_TYPE_STRING,
    FIELD_DATA_TYPE_VARCHAR,
    FIELD_DATA_TYPE_BOOL,
};

struct TableField {
    char *name;
    enum TableFieldDataType data_type;
    size_t data_size;
};

struct TableSchema {
    size_t fields_length;
    struct TableField fields[];
};

#endif //LLP_DATABASE_TABLE_SCHEMA_H
