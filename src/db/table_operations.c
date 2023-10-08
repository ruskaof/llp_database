//
// Created by ruskaof on 2/10/23.
//

#include "../../include/table_operations.h"
#include "../../include/data_operations.h"
#include "../utils/logging.h"
#include "table_metadata.h"
#include "element_allocator.h"
#include "file.h"

#include <string.h>

uint64_t get_table_metadata_element_size(struct TableMetadataElement *table_metadata_element) {
    return sizeof(struct TableMetadataElement) + table_metadata_element->columns_count * sizeof(struct TableColumn);
}

int operation_create_table(char *table_name, struct TableColumn *columns, uint64_t columns_count) {
    if (columns_count == 0) {
        logger(LL_ERROR, __func__, "Cannot create table with 0 columns");
        return -1;
    }

    if (strlen(table_name) > MAX_TABLE_NAME_LENGTH) {
        logger(LL_ERROR, __func__, "Cannot create table with name longer than %d", MAX_TABLE_NAME_LENGTH);
        return -1;
    }

    uint64_t existing_table_metadata_offset;
    if (find_table_metadata_offset(table_name, &existing_table_metadata_offset) == 0) {
        logger(LL_ERROR, __func__, "Cannot create table with name %s because it already exists", table_name);
        return -1;
    }

    struct TableMetadataElement *table_metadata_element = malloc(sizeof(struct TableMetadataElement) +
                                                                 columns_count * sizeof(struct TableColumn));
    strcpy(table_metadata_element->name, table_name);
    table_metadata_element->has_rows = false;
    table_metadata_element->columns_count = columns_count;
    memcpy(table_metadata_element->columns, columns, columns_count * sizeof(struct TableColumn));

    uint64_t table_metadata_element_size = get_table_metadata_element_size(table_metadata_element);
    uint64_t table_metadata_element_offset;
    int allocate_result = allocate_element(table_metadata_element_size, ET_TABLE_METADATA,
                                           &table_metadata_element_offset);

    if (allocate_result == -1) {
        logger(LL_ERROR, __func__, "Cannot create table with name %s because of allocation error", table_name);
        return -1;
    }

    void *file_data_pointer;
    int mmap_result = mmap_file(&file_data_pointer, 0, get_file_size());
    if (mmap_result == -1) {
        logger(LL_ERROR, __func__, "Cannot mmap file");
        return -1;
    }
    memcpy((char *) file_data_pointer + table_metadata_element_offset + ELEMENT_VALUE_OFFSET,
           table_metadata_element, table_metadata_element_size);

    free(table_metadata_element);


    return 0;
}

int operation_drop_table(char *table_name) {
    int truncate_result = operation_truncate(table_name);
    if (truncate_result == -1) {
        logger(LL_ERROR, __func__, "Cannot drop table with name %s because of truncation error", table_name);
        return -1;
    }

    uint64_t table_metadata_element_offset;
    if (find_table_metadata_offset(table_name, &table_metadata_element_offset) == -1) {
        logger(LL_ERROR, __func__, "Cannot drop table with name %s because it does not exist", table_name);
        return -1;
    }

    int delete_result = delete_element(table_metadata_element_offset);
    if (delete_result == -1) {
        logger(LL_ERROR, __func__, "Cannot drop table with name %s because of deletion error", table_name);
        return -1;
    }

    return 0;
}
