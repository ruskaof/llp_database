//
// Created by ruskaof on 1/10/23.
//

#ifndef LLP_DATABASE_TABLE_H
#define LLP_DATABASE_TABLE_H

#include <stddef.h>

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
    struct TableField fields[];
};

struct TableColumn {
    TableColumnSchemaName name;
    enum TableDatatype type;
};

#endif //LLP_DATABASE_TABLE_H
