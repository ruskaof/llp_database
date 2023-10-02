//
// Created by ruskaof on 2/10/23.
//

#ifndef LLP_DATABASE_TABLE_METADATA_H
#define LLP_DATABASE_TABLE_METADATA_H

#include "../../include/table.h"

#include <stdint.h>

struct TableMetadata {
    TableSchemaName name;
    uint64_t first_page_offset;
    uint64_t columns_count;
    struct TableColumn columns[];
};

int find_table_metadata(int fd, char *table_name, uint64_t *table_metadata_offset, uint64_t *table_metadata_size);

#endif //LLP_DATABASE_TABLE_METADATA_H
