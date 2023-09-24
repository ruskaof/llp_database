//
// Created by ruskaof on 24/09/23.
//

#ifndef LLP_DATABASE_TABLE_DEFINITION_OPERATIONS_H
#define LLP_DATABASE_TABLE_DEFINITION_OPERATIONS_H

#include "table_schema.h"
#include "db.h"

#include <stdlib.h>

void operation_create_table(
    char *table_name,
    size_t fields_length,
    struct TableFieldSchema *fields,
    struct OpenedDbInfo opened_db_info
);

void operation_drop_table(char *table_name);

#endif //LLP_DATABASE_TABLE_DEFINITION_OPERATIONS_H
