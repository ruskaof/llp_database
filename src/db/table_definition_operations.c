//
// Created by ruskaof on 24/09/23.
//

#include "../../include/table_definition_operations.h"
#include "./hash_table/hash_table.h"
#include "../utils/logging.h"


void operation_create_table(
    char *table_name,
    size_t fields_length,
    struct TableFieldSchema *fields,
    struct OpenedDbInfo opened_db_info
) {
    logger(LL_DEBUG, __func__, "Creating table with name: %s", table_name);

    struct HashTable *hash_table = (struct HashTable *) opened_db_info.mmaped_file;

    struct TablePage *table_page = get_last_table_page(hash_table, table_name);
    if (table_page != NULL) {
        logger(LL_DEBUG, __func__, "Table with name: %s already exists.", table_name);
        return;
    }

    // create page for table
    table_page = create_table_page(table_name, fields_length, fields);

    insert_or_update_page(hash_table, table_name, table_page);
}
