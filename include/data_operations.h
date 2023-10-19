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
    TableColumnSchemaName column_name;
    enum PredicateOperator predicate_operator;
    uint64_t value_size;
    void *value;
};

struct OperationPredicate {
    uint64_t parameter_count;
    struct OperationPredicateParameter *parameters;
};

struct SelectResultIterator;

struct SelectResultIterator operation_select(TableSchemaName table_name, struct OperationPredicate *predicate);

struct TableRow get_by_iterator(struct SelectResultIterator *iterator);

bool next(struct SelectResultIterator *iterator);

#endif //LLP_DATABASE_DATA_OPERATIONS_H
