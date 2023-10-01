//
// Created by ruskaof on 1/10/23.
//

#ifndef LLP_DATABASE_TABLE_METADATA_H
#define LLP_DATABASE_TABLE_METADATA_H

#include "../../include/table.h"
#include "file.h"

#include <stddef.h>
#include <stdbool.h>


struct TableMetadata {
    TableSchemaName name;
    off_t first_page_offset;
    size_t columns_count;
    struct TableColumn columns[];
};

int insert_table_metadata_to_file(int fd, const struct TableMetadata *table_schema);

int delete_table_schema(int fd, const char *name);

struct TableMetadata *get_table_schema_from_file(int fd, const char *name);

#endif //LLP_DATABASE_TABLE_METADATA_H
