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

struct FileHeader {
    bool has_deleted_pages;
    uint64_t first_deleted_page_offset;
    bool has_table_metadata_pages;
    uint64_t last_table_metadata_page_offset;
    bool has_table_data_pages;
    uint64_t last_table_data_page_offset;
};

#define FIRST_PAGE_OFFSET sizeof(struct FileHeader)

enum PageType {
    PT_DELETED_PAGE,
    PT_TABLE_METADATA_PAGE,
    PT_TABLE_DATA_PAGE
};

struct PageHeader {
    bool has_elements;
    /**
     * 0 - page is first of its type
     */
    uint64_t prev_page_of_type_offset;
    /**
     * 0 - page is last of its type
     */
    uint64_t next_page_of_type_offset;
    uint64_t page_size;
    enum PageType page_type;
};

struct TablePageSubHeader {
    TableSchemaName table_name;
};

struct TableMetadataPageSubHeader {

};

struct PageItem {
    uint64_t item_size;
    bool is_last;
    void *item_data;
};

#endif //LLP_DATABASE_PAGE_H
