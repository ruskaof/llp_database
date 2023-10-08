//
// Created by ruskaof on 1/10/23.
//

#ifndef LLP_DATABASE_DATA_OPERATIONS_H
#define LLP_DATABASE_DATA_OPERATIONS_H

#include "table.h"

struct SelectResultIterator {
    bool has_element;
    bool has_more;
    uint64_t current_element_offset;
};

enum PredicateOperator {
    PO_EQUAL,
    PO_NOT_EQUAL,
    PO_GREATER_THAN,
    PO_GREATER_THAN_OR_EQUAL,
    PO_LESS_THAN,
    PO_LESS_THAN_OR_EQUAL
};

struct OperationPredicateParameter {
    struct OperationPredicateParameter *next;
    TableColumnSchemaName column_name;
    enum PredicateOperator predicate_operator;
    uint64_t value_size;
    void *value;
};

struct SelectResultIterator
operation_select(char *table_name, uint64_t parameters_count, struct OperationPredicateParameter *parameters);

int operation_insert(char *table_name, struct TableField *first_table_field);

/**
 *
 * @param table_name The name of the table to delete from.
 * @param parameters Can be NULL if no parameters are needed.
 * @return 0 if the operation was successful, -1 otherwise.
 */
//int operation_delete(char *table_name, struct OperationPredicateParameters *parameters);

/**
 *
 * @param table_name The name of the table to update.
 * @param parameters Can be NULL if no parameters are needed.
 * @return 0 if the operation was successful, -1 otherwise.
 */
//int operation_update(char *table_name, struct OperationPredicateParameters *parameters);

#endif //LLP_DATABASE_DATA_OPERATIONS_H
