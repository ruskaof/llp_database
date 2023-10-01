//
// Created by ruskaof on 1/10/23.
//

#ifndef LLP_DATABASE_SCHEMA_H
#define LLP_DATABASE_SCHEMA_H

#include "../src/db/file.h"

#include <stddef.h>
#include <stdbool.h>

#define MAX_TABLE_NAME_LENGTH 255
#define MAX_TABLE_COLUMN_NAME_LENGTH 255

typedef char TableSchemaName[MAX_TABLE_NAME_LENGTH];
typedef char TableColumnSchemaName[MAX_TABLE_COLUMN_NAME_LENGTH];

enum TableDatatype {
    TD_INT64,
    TD_DOUBLE,
    TD_STRING,
    TD_BOOL,
};

struct TableField {
    enum TableDatatype type;
    char *name;
};

struct TableRow {
    size_t fields_count;
    struct TableField *fields;
};

struct TableColumn {
    TableColumnSchemaName name;
    enum TableDatatype type;
};

struct __attribute__((__packed__)) TableSchema {
    TableSchemaName name;
    size_t columns_count;
    struct TableColumn columns[];
};

size_t get_table_schema_size(const struct TableSchema *table_schema);

int insert_table_schema_to_file(int fd, const struct TableSchema *table_schema);

int delete_table_schema(int fd, const char *name);

struct TableSchema *get_table_schema_from_file(int fd, const char *name);

#endif //LLP_DATABASE_SCHEMA_H
