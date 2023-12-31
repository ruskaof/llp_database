//
// Created by ruskaof on 1/10/23.
//

#ifndef LLP_DATABASE_DATA_OPERATIONS_H
#define LLP_DATABASE_DATA_OPERATIONS_H

#include "table.h"

enum PredicateOperator {
    PO_EQUAL,
    PO_NOT_EQUAL,
    PO_GREATER_THAN,
    PO_LESS_THAN,
};

struct OperationPredicateParameter {
    struct OperationPredicateParameter *next;
    TableColumnSchemaName column_name;
    enum PredicateOperator predicate_operator;
    uint64_t value_size;
    void *value;
};

struct SelectResultIterator
operation_select(char *table_name, struct OperationPredicateParameter *parameters);

struct SelectResultIterator {
    bool has_element;
    bool has_more;
    uint64_t current_element_offset;
    uint64_t table_metadata_offset;
    struct OperationPredicateParameter *parameters;
};

struct TableField *get_by_iterator(struct SelectResultIterator *iterator);

struct SelectResultIterator get_next(struct SelectResultIterator *iterator);

int operation_insert(char *table_name, struct TableField *first_table_field);

int operation_truncate(char *table_name);

int operation_delete(char *table_name, struct OperationPredicateParameter *parameters);

int operation_update(char *table_name,
                     struct OperationPredicateParameter *parameters,
                     struct TableField *first_table_field);

#endif //LLP_DATABASE_DATA_OPERATIONS_H
