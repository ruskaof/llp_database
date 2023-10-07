//
// Created by ruskaof on 2/10/23.
//

#ifndef LLP_DATABASE_TABLE_METADATA_H
#define LLP_DATABASE_TABLE_METADATA_H

#include "../../include/table.h"

#include <stdint.h>
#include <stdbool.h>

struct TableMetadataElement {
    TableSchemaName name;
    bool has_rows;
    uint64_t last_row_offset;
    uint64_t columns_count;
    struct TableColumn columns[];
};

int find_table_metadata_offset(int fd, char *table_name, uint64_t *table_metadata_offset);



#endif //LLP_DATABASE_TABLE_METADATA_H
