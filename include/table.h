//
// Created by ruskaof on 1/10/23.
//

#ifndef LLP_DATABASE_TABLE_H
#define LLP_DATABASE_TABLE_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#define MAX_TABLE_NAME_LENGTH 255
#define MAX_TABLE_COLUMN_NAME_LENGTH 255

typedef char TableSchemaName[MAX_TABLE_NAME_LENGTH];
typedef char TableColumnSchemaName[MAX_TABLE_COLUMN_NAME_LENGTH];

enum TableDatatype {
    TD_INT64,
    TD_FLOAT64,
    TD_STRING,
    TD_BOOL,
};

struct TableField {
    uint64_t size;
    struct TableField *next;
    void *value;
};

struct TableColumn {
    TableColumnSchemaName name;
    enum TableDatatype type;
};

#endif //LLP_DATABASE_TABLE_H
