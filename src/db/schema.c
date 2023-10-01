//
// Created by ruskaof on 1/10/23.
//

#include "schema.h"
#include "../utils/logging.h"
#include "page.h"

#include <stdbool.h>
#include <string.h>

#define TABLE_SCHEMA_TO_DELETE_NOT_FOUND "Table schema to delete not found."
#define TABLE_SCHEMA_TO_GET_NOT_FOUND "Table schema to get not found."

struct TableMetadataNode {
    bool is_last;
    struct TableMetadata table_metadata;
};

size_t get_table_metadata_size(const struct TableMetadata *table_schema) {
    size_t size = sizeof(struct TableMetadata) + table_schema->columns_count * sizeof(struct TableColumn);
    return size;
}

size_t get_table_metadata_node_size(const struct TableMetadataNode *table_schema_node) {
    size_t size = sizeof(struct TableMetadataNode) + get_table_metadata_size(&table_schema_node->table_metadata);
    return size;
}

struct TableMetadataNode *get_next_table_schema_node(struct TableMetadataNode *table_metadata_node) {
    if (table_metadata_node->is_last) {
        return NULL;
    }

    return (struct TableMetadataNode *) (((char *) table_metadata_node) +
                                         get_table_metadata_node_size(table_metadata_node));
}

struct TableMetadataNode *get_last_table_metadata_node(struct TableMetadataNode *table_metadata_node) {
    if (table_metadata_node->is_last) {
        return table_metadata_node;
    }

    while (table_metadata_node->is_last != true) {
        table_metadata_node = get_next_table_schema_node(table_metadata_node);
    }

    return table_metadata_node;
}

int insert_table_metadata_to_file(int fd, const struct TableMetadata *table_schema) {
    logger(LL_DEBUG, __func__, "Inserting table schema %s.", table_schema->name);

    size_t file_size = get_file_size(fd);

    if (file_size == 0) {
        logger(LL_DEBUG, __func__, "File is empty. Inserting table schema %s.", table_schema->name);

        change_file_size(fd, TABLE_SCHEMA_PAGE_SIZE);
        file_size = TABLE_SCHEMA_PAGE_SIZE;
    }

    void *file_data_pointer;
    if (mmap_file(fd, &file_data_pointer, TABLE_SCHEMA_PAGE_OFFSET, TABLE_SCHEMA_PAGE_SIZE) != 0) {
        return -1;
    }

    struct PageHeader *page_header = (struct PageHeader *) file_data_pointer;
    struct TableMetadataNode *table_schema_first_node = (struct TableMetadataNode *) (file_data_pointer +
                                                                                      sizeof(struct PageHeader));

    if (page_header->has_elements != true) {
        table_schema_first_node->is_last = true;
        memcpy(&table_schema_first_node->table_metadata, table_schema, get_table_metadata_size(table_schema));
    } else {
        struct TableMetadataNode *table_schema_node = table_schema_first_node;

        while (table_schema_node->is_last != true) {
            table_schema_node = get_next_table_schema_node(table_schema_node);
        }

        struct TableMetadataNode *new_table_schema_node = (struct TableMetadataNode *) (((char *) table_schema_node) +
                                                                                        get_table_metadata_node_size(
                                                                                            table_schema_node));
        new_table_schema_node->is_last = true;
        memcpy(&new_table_schema_node->table_metadata, table_schema, get_table_metadata_size(table_schema));
        table_schema_node->is_last = false;
    }

    page_header->has_elements = true;

    if (sync_file(fd) != 0) {
        return -1;
    }

    if (munmap_file(file_data_pointer, file_size) != 0) {
        return -1;
    }

    return 0;
}

struct TableMetadata *get_table_schema_from_file(int fd, const char *name) {
    logger(LL_DEBUG, __func__, "Getting table schema %s.", name);

    size_t file_size = get_file_size(fd);

    if (file_size == 0) {
        logger(LL_ERROR, __func__, "Could not get table schema %s. File is empty.", name);
        return NULL;
    }

    void *file_data_pointer;
    if (mmap_file(fd, &file_data_pointer, TABLE_SCHEMA_PAGE_OFFSET, TABLE_SCHEMA_PAGE_SIZE) != 0) {
        return NULL;
    }

    struct PageHeader *page_header = (struct PageHeader *) file_data_pointer;
    struct TableMetadataNode *table_schema_first_node = (struct TableMetadataNode *) (file_data_pointer +
                                                                                      sizeof(struct PageHeader));

    if (page_header->has_elements != true) {
        logger(LL_WARN, __func__, TABLE_SCHEMA_TO_GET_NOT_FOUND);

        if (munmap_file(file_data_pointer, file_size) != 0) {
            return NULL;
        }

        return NULL;
    }

    struct TableMetadataNode *table_schema_node = table_schema_first_node;

    while (table_schema_node->is_last != true) {
        if (strcmp(table_schema_node->table_metadata.name, name) == 0) {
            struct TableMetadata *table_schema = malloc(get_table_metadata_size(&table_schema_node->table_metadata));
            memcpy(table_schema, &table_schema_node->table_metadata,
                   get_table_metadata_size(&table_schema_node->table_metadata));

            if (munmap_file(file_data_pointer, file_size) != 0) {
                return NULL;
            }

            return table_schema;
        }

        table_schema_node = get_next_table_schema_node(table_schema_node);
    }

    if (strcmp(table_schema_node->table_metadata.name, name) == 0) {
        struct TableMetadata *table_schema = malloc(get_table_metadata_size(&table_schema_node->table_metadata));
        memcpy(table_schema, &table_schema_node->table_metadata,
               get_table_metadata_size(&table_schema_node->table_metadata));

        if (munmap_file(file_data_pointer, file_size) != 0) {
            return NULL;
        }

        return table_schema;
    }

    logger(LL_WARN, __func__, TABLE_SCHEMA_TO_GET_NOT_FOUND);

    if (munmap_file(file_data_pointer, file_size) != 0) {
        return NULL;
    }

    return NULL;

}

int delete_table_schema(int fd, const char *name) {
    logger(LL_DEBUG, __func__, "Deleting table schema %s.", name);


    size_t file_size = get_file_size(fd);

    if (file_size == 0) {
        logger(LL_ERROR, __func__, "Could not get delete schema %s. File is empty.", name);
        return -1;
    }

    void *file_data_pointer;
    if (mmap_file(fd, &file_data_pointer, TABLE_SCHEMA_PAGE_OFFSET, TABLE_SCHEMA_PAGE_SIZE) != 0) {
        return -1;
    }

    struct PageHeader *page_header = (struct PageHeader *) file_data_pointer;
    struct TableMetadataNode *table_schema_first_node = (struct TableMetadataNode *) (file_data_pointer +
                                                                                      sizeof(struct PageHeader));

    if (page_header->has_elements != true) {
        logger(LL_WARN, __func__, TABLE_SCHEMA_TO_DELETE_NOT_FOUND);

        if (munmap_file(file_data_pointer, file_size) != 0) {
            return -1;
        }

        return -1;
    }

    if (strcmp(table_schema_first_node->table_metadata.name, name) == 0 && table_schema_first_node->is_last == true) {
        page_header->has_elements = false;

        if (sync_file(fd) != 0) {
            return -1;
        }

        if (munmap_file(file_data_pointer, file_size) != 0) {
            return -1;
        }

        return 0;
    }

    struct TableMetadataNode *table_schema_node = table_schema_first_node;

    while (table_schema_node->is_last != true) {
        if (strcmp(table_schema_node->table_metadata.name, name) == 0) {
            size_t amount_of_bytes_to_move =
                (char *) get_last_table_metadata_node(table_schema_node) - (char *) table_schema_node;
            memcpy(table_schema_node, get_next_table_schema_node(table_schema_node), amount_of_bytes_to_move);

            if (sync_file(fd) != 0) {
                return -1;
            }

            if (munmap_file(file_data_pointer, file_size) != 0) {
                return -1;
            }

            return 0;
        }

        table_schema_node = get_next_table_schema_node(table_schema_node);
    }

    if (strcmp(table_schema_node->table_metadata.name, name) == 0) {
        size_t amount_of_bytes_to_move =
            (char *) get_last_table_metadata_node(table_schema_node) - (char *) table_schema_node;
        memcpy(table_schema_node, get_next_table_schema_node(table_schema_node), amount_of_bytes_to_move);

        if (sync_file(fd) != 0) {
            return -1;
        }

        if (munmap_file(file_data_pointer, file_size) != 0) {
            return -1;
        }

        return 0;
    }

    logger(LL_WARN, __func__, TABLE_SCHEMA_TO_DELETE_NOT_FOUND);

    if (munmap_file(file_data_pointer, file_size) != 0) {
        return -1;
    }

    return -1;
}
