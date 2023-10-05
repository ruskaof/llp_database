//
// Created by ruskaof on 1/10/23.
//

#ifndef LLP_DATABASE_ELEMENT_H
#define LLP_DATABASE_ELEMENT_H

#include "../../include/table.h"

#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>

#define ALLOC_SIZE 65536
struct FileHeader {
    bool has_deleted_elements;
    uint64_t first_deleted_element_offset;
    bool has_table_metadata_elements;
    uint64_t last_table_metadata_element_offset;
    bool has_table_data_elements;
    uint64_t last_table_data_element_offset;
    uint64_t last_element_offset;
};

#define FIRST_ELEMENT_OFFSET sizeof(struct FileHeader)

enum ElementType {
    ET_DELETED,
    ET_TABLE_METADATA,
    ET_TABLE_DATA
};

struct ElementHeader {
    uint64_t element_size;
    enum ElementType element_type;
    bool has_next_element_of_type;
    uint64_t next_element_of_type_offset;
    bool has_prev_element_of_type;
    uint64_t prev_element_of_type_offset;
    bool has_prev_element;
    uint64_t prev_element_offset;
};

#define MIN_ELEMENT_SIZE sizeof (struct ElementHeader) + 1024

#endif //LLP_DATABASE_ELEMENT_H
