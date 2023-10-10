//
// Created by ruskaof on 2/10/23.
//

#ifndef LLP_DATABASE_TABLE_OPERATIONS_H
#define LLP_DATABASE_TABLE_OPERATIONS_H

#include "table.h"
#include "data_operations.h"

#include <stdint.h>

int operation_create_table(char *table_name, struct TableColumn *columns, uint64_t columns_count);

int operation_inner_join(char *left_table_name,
                         struct OperationPredicateParameter *left_table_parameters,
                         char *left_table_column_name,
                         char *right_table_name,
                         struct OperationPredicateParameter *right_table_parameters,
                         char *right_table_column_name,
                         char *result_table_name);

int operation_drop_table(char *table_name);

#endif //LLP_DATABASE_TABLE_OPERATIONS_H
