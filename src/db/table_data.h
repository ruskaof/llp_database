//
// Created by ruskaof on 8/10/23.
//

#ifndef LLP_DATABASE_TABLE_DATA_H
#define LLP_DATABASE_TABLE_DATA_H

#include <stdint.h>
#include <stdbool.h>

/**
 * Value must be allocated just after the TableDataElement struct
 */
struct TableDataFieldElement {
    bool has_next;
    uint64_t next_field_offset;
    uint64_t field_size;
};

struct TableDataElement {
    bool has_prev_of_table;
    uint64_t prev_of_table_offset;
};

#define TABLE_DATA_ELEMENT_FIRST_FIELD_OFFSET sizeof(struct TableDataElement)

#endif //LLP_DATABASE_TABLE_DATA_H
