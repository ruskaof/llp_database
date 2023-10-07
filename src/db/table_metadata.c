//
// Created by ruskaof on 7/10/23.
//

#include <string.h>
#include "table_metadata.h"
#include "../utils/logging.h"
#include "file.h"
#include "element.h"

int find_table_metadata_offset(int fd, char *table_name, uint64_t *table_metadata_offset) {
    uint64_t file_size = get_file_size(fd);

    if (file_size == 0) {
        logger(LL_DEBUG, __func__, "Cannot find table metadata offset in empty file");
        return -1;
    }

    void *file_data_pointer;
    int mmap_result = mmap_file(fd, &file_data_pointer, 0, file_size);
    if (mmap_result == -1) {
        logger(LL_ERROR, __func__, "Cannot mmap file");
        return -1;
    }

    struct FileHeader *file_header = file_data_pointer;
    if (!file_header->has_table_metadata_elements) {
        logger(LL_ERROR, __func__, "Cannot find table metadata offset in file without table metadata elements");
        return -1;
    }

    uint64_t current_table_metadata_element_offset = file_header->last_table_metadata_element_offset;
    struct ElementHeader *element_header =
        (struct ElementHeader *) ((char *) file_data_pointer + current_table_metadata_element_offset);
    struct TableMetadataElement *table_metadata_element = (struct TableMetadataElement *) ((char *) element_header +
                                                                                           ELEMENT_SUBHEADER_OFFSET);
    if (strcmp(table_metadata_element->name, table_name) == 0) {
        logger(LL_INFO, __func__, "Found table metadata element with name %s at offset %ld", table_name,
               current_table_metadata_element_offset);
        *table_metadata_offset = current_table_metadata_element_offset;
        return 0;
    }

    while (element_header->has_prev_element_of_type) {
        current_table_metadata_element_offset = element_header->prev_element_of_type_offset;
        element_header =
            (struct ElementHeader *) ((char *) file_data_pointer + current_table_metadata_element_offset);
        table_metadata_element = (struct TableMetadataElement *) ((char *) element_header + ELEMENT_SUBHEADER_OFFSET);

        if (strcmp(table_metadata_element->name, table_name) == 0) {
            logger(LL_INFO, __func__, "Found table metadata element with name %s at offset %ld", table_name,
                   current_table_metadata_element_offset);
            *table_metadata_offset = current_table_metadata_element_offset;
            return 0;
        }
    }

    logger(LL_INFO, __func__, "Cannot find table metadata element with name %s", table_name);

    return -1;
}