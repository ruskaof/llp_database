//
// Created by ruskaof on 2/10/23.
//

#ifndef LLP_DATABASE_TABLE_OPERATIONS_H
#define LLP_DATABASE_TABLE_OPERATIONS_H

#include "table.h"

#include <stdint.h>

int operation_create_table(int fd, char *table_name, struct TableColumn *columns, uint64_t columns_count);

int operation_drop_table(char *table_name);

#endif //LLP_DATABASE_TABLE_OPERATIONS_H
