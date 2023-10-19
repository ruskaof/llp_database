//
// Created by ruskaof on 10/19/23.
//

#include "../../include/table_operations.h"
#include "db.h"
#include "elements.h"
#include "../utils/logging.h"

int operation_create_table(TableSchemaName table_name, uint64_t column_count, struct TableColumn *columns) {
    uint64_t current_page_offset = FIRST_METADATA_PAGE_OFFSET;
    struct PageHeader *current_page_header = NULL;

    if (!has_first_page()) {
        current_page_offset = allocate_page();
        current_page_header = mmap_page_header(current_page_offset);
        current_page_header->has_next_page = false;
        struct MetadataPageHeader *metadata_page_header = (void *) current_page_header + sizeof(struct PageHeader);
        metadata_page_header->has_elements = false;
    } else {
        current_page_header = mmap_page_header(current_page_offset);
    }

    if (current_page_header == NULL) {
        logger(LL_ERROR, __func__, "Could not get metadata page header.");
        return -1;
    }

    struct MetadataPageHeader *metadata_page_header = (void *) current_page_header + sizeof(struct PageHeader);

    if (metadata_page_header->has_elements)
}

int operation_drop_table(TableSchemaName table_name);
