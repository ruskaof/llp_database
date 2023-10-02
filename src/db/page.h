//
// Created by ruskaof on 1/10/23.
//

#ifndef LLP_DATABASE_PAGE_H
#define LLP_DATABASE_PAGE_H

#include "../../include/table.h"

#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>

#define MIN_PAGE_SIZE 65536
#define TABLE_SCHEMA_PAGE_SIZE 65536

enum PageType {
    PT_TABLE_METADATA_PAGE,
    PT_TABLE_DATA_PAGE
};

struct PageHeader {
    bool has_elements;
    bool is_deleted;
    uint64_t page_size;
    enum PageType page_type;
};

struct TablePageSubHeader {
    TableSchemaName table_name;
};

struct TableMetadataPageSubHeader {

};

#endif //LLP_DATABASE_PAGE_H
