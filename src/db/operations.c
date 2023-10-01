//
// Created by ruskaof on 1/10/23.
//

#include "../../include/operations.h"
#include "../utils/logging.h"
#include "page.h"

struct TableRowNode {
    bool is_last;
    struct TableRow row;
};

size_t get_table_row_size(const struct TableRow *row) {
    size_t size = sizeof(struct TableRow) + row->fields_count * sizeof(struct TableRow);
    return size;
}

size_t get_table_row_node_size(const struct TableRowNode *row_node) {
    size_t size = sizeof(struct TableRowNode) + get_table_row_size(&row_node->row);
    return size;
}

struct TableRowNode *get_next_table_row_node(struct TableRowNode *row_node) {
    if (row_node->is_last) {
        return NULL;
    }

    return (struct TableRowNode *) (((char *) row_node) + get_table_row_node_size(row_node));
}

struct SelectResultIterator *operation_select(char *table_name, struct OperationPredicateParameters *parameters) {
    logger(LL_DEBUG, __func__, "Selecting from table %s.", table_name);


}

int operation_insert(char *table_name, struct TableRow *row) {

}