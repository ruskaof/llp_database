//
// Created by ruskaof on 1/10/23.
//

#ifndef LLP_DATABASE_DATA_OPERATIONS_H
#define LLP_DATABASE_DATA_OPERATIONS_H

struct SelectResultIterator {
    struct TableRow *current_row;
};

struct OperationPredicateParameter {
    char *column_name;
    void *value;
};


/**
 *
 * @param table_name The name of the table to select from.
 * @param parameters Can be NULL if no parameters are needed.
 * @return A SelectResultIterator that can be used to iterate over the results.
 */
//struct SelectResultIterator *operation_select(char *table_name, struct OperationPredicateParameters *parameters);

/**
 *
 * @param table_name The name of the table to insert into.
 * @param row The row to insert.
 * @return 0 if the operation was successful, -1 otherwise.
 */
int operation_insert(char *table_name, struct TableRow *row);

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
