//
// Created by ruskaof on 2/10/23.
//

#ifndef LLP_DATABASE_TABLE_OPERATIONS_H
#define LLP_DATABASE_TABLE_OPERATIONS_H

#include "table.h"
#include "data_operations.h"

#include <stdint.h>

int operation_create_table(TableSchemaName table_name, uint64_t column_count, struct TableColumn *columns);

int operation_drop_table(TableSchemaName table_name);

#endif //LLP_DATABASE_TABLE_OPERATIONS_H
