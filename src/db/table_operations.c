//
// Created by ruskaof on 2/10/23.
//

#include "../../include/table_operations.h"
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

    memcpy((char *) get_file_data_pointer() + table_metadata_element_offset + ELEMENT_VALUE_OFFSET,
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

uint64_t get_column_index_by_name(struct TableMetadataElement *table_metadata_element, char *column_name) {
    for (uint64_t i = 0; i < table_metadata_element->columns_count; i++) {
        if (strcmp(table_metadata_element->columns[i].name, column_name) == 0) {
            return i;
        }
    }

    return -1;
}

int operation_inner_join(char *left_table_name,
                         struct OperationPredicateParameter *left_table_parameters,
                         char *left_table_column_name,
                         char *right_table_name,
                         struct OperationPredicateParameter *right_table_parameters,
                         char *right_table_column_name,
                         char *result_table_name) {
    uint64_t left_table_metadata_element_offset;
    if (find_table_metadata_offset(left_table_name, &left_table_metadata_element_offset) == -1) {
        logger(LL_ERROR, __func__, "Cannot join table with name %s because it does not exist", left_table_name);
        return -1;
    }

    uint64_t right_table_metadata_element_offset;
    if (find_table_metadata_offset(right_table_name, &right_table_metadata_element_offset) == -1) {
        logger(LL_ERROR, __func__, "Cannot join table with name %s because it does not exist", right_table_name);
        return -1;
    }

    // create new metadata
    struct TableMetadataElement *left_table_metadata_element =
        get_file_data_pointer() + left_table_metadata_element_offset + ELEMENT_VALUE_OFFSET;

    struct TableMetadataElement *right_table_metadata_element =
        get_file_data_pointer() + right_table_metadata_element_offset + ELEMENT_VALUE_OFFSET;

    uint64_t result_table_columns_count = left_table_metadata_element->columns_count +
                                          right_table_metadata_element->columns_count;

    struct TableColumn *result_table_columns = malloc(result_table_columns_count * sizeof(struct TableColumn));

    for (uint64_t i = 0; i < left_table_metadata_element->columns_count; i++) {
        result_table_columns[i] = left_table_metadata_element->columns[i];
    }

    for (uint64_t i = 0; i < right_table_metadata_element->columns_count; i++) {
        result_table_columns[left_table_metadata_element->columns_count + i] =
            right_table_metadata_element->columns[i];
    }

    int create_table_result = operation_create_table(result_table_name, result_table_columns,
                                                     result_table_columns_count);

    if (create_table_result == -1) {
        logger(LL_ERROR, __func__, "Cannot join table with name %s and %s because of table creation error",
               left_table_name, right_table_name);
        return -1;
    }

    free(result_table_columns);

    struct SelectResultIterator left_table_iterator =
        operation_select(left_table_name, left_table_parameters);


    uint64_t left_column_index = get_column_index_by_name(left_table_metadata_element, left_table_column_name);
    uint64_t right_column_index = get_column_index_by_name(right_table_metadata_element, right_table_column_name);

    while (left_table_iterator.has_element) {
        struct TableField *first_left_table_field = get_by_iterator(&left_table_iterator);
        struct TableField *current_left_table_field = first_left_table_field;
        struct SelectResultIterator right_table_iterator =
            operation_select(right_table_name, right_table_parameters);

        for (uint64_t i = 0; i < left_table_metadata_element->columns_count; i++) {
            if (i == left_column_index) {
                break;
            }

            current_left_table_field = current_left_table_field->next;
        }

        while (right_table_iterator.has_element) {
            struct TableField *first_right_table_field = get_by_iterator(&right_table_iterator);
            struct TableField *current_right_table_field = first_right_table_field;

            for (uint64_t i = 0; i < right_table_metadata_element->columns_count; i++) {
                if (i == right_column_index) {
                    break;
                }

                current_right_table_field = current_right_table_field->next;
            }

            if (current_left_table_field->size == current_right_table_field->size &&
                memcmp(current_left_table_field->value, current_right_table_field->value,
                       current_left_table_field->size) == 0) {
                struct TableField *last_left_table_field = current_left_table_field;
                while (last_left_table_field->next != NULL) {
                    last_left_table_field = last_left_table_field->next;
                }

                last_left_table_field->next = first_right_table_field;

                int insert_result = operation_insert(result_table_name, first_left_table_field);
                if (insert_result == -1) {
                    logger(LL_ERROR, __func__, "Cannot join table with name %s and %s because of insertion error",
                           left_table_name, right_table_name);
                    return -1;
                }

                last_left_table_field->next = NULL;

                free_table_row(first_right_table_field);
                break;
            }

            free_table_row(first_right_table_field);

            right_table_iterator = get_next(&right_table_iterator);
        }

        free_table_row(first_left_table_field);

        left_table_iterator = get_next(&left_table_iterator);
    }

    return 0;
}
