//
// Created by ruskaof on 2/10/23.
//

#include "../../include/table_operations.h"
#include "../utils/logging.h"
#include "table_metadata.h"
#include "allocator.h"
#include "file.h"

#include <string.h>

struct TableMetadataLLNode {
    bool is_last;
    struct TableMetadata table_metadata;
};

uint64_t get_table_metadata_size(struct TableMetadata *table_metadata) {
    return sizeof(struct TableMetadata) + table_metadata->columns_count * sizeof(struct TableColumn);
}

uint64_t get_table_metadata_ll_node_size(struct TableMetadataLLNode *table_metadata_ll_node) {
    return sizeof(struct TableMetadataLLNode) + get_table_metadata_size(&table_metadata_ll_node->table_metadata);
}

int find_table_metadata(int fd, char *table_name, uint64_t *table_metadata_offset, uint64_t *table_metadata_size) {
    uint64_t file_size = get_file_size(fd);

    void *file_data_pointer;
    int mmap_res = mmap_file(fd, &file_data_pointer, 0, file_size);
    if (mmap_res != 0) {
        logger(LL_ERROR, __func__, "Failed to mmap file");
        return -1;
    }

    uint64_t current_offset = 0;

    while (current_offset < file_size) {
        struct PageHeader *page_header = (struct PageHeader *) ((char *) file_data_pointer + current_offset);

        if (page_header->page_type == PT_TABLE_METADATA_PAGE) {

        }
    }
}

int operation_create_table(int fd, char *table_name, struct TableColumn *columns, uint64_t columns_count) {
    logger(LL_DEBUG, __func__, "Creating table %s", table_name);

    uint64_t table_data_page_offset;
    uint64_t table_data_page_size;
    int table_data_page_alloc_res = allocate_page(fd,
                                                  MIN_PAGE_SIZE,
                                                  PT_TABLE_DATA_PAGE,
                                                  &table_data_page_offset,
                                                  &table_data_page_size);
    if (table_data_page_alloc_res != 0) {
        logger(LL_ERROR, __func__, "Failed to allocate page for table data");
        return -1;
    }

    struct TableMetadata *table_metadata = malloc(
        sizeof(struct TableMetadata) + columns_count * sizeof(struct TableColumn));
    strcpy(table_metadata->name, table_name);
    table_metadata->first_page_offset = table_data_page_offset;
    table_metadata->columns_count = columns_count;
    memcpy(table_metadata->columns, columns, columns_count * sizeof(struct TableColumn));

    // find table metadata page
}