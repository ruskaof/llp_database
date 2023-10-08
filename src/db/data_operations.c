//
// Created by ruskaof on 1/10/23.
//

#include <string.h>
#include "../../include/data_operations.h"
#include "../utils/logging.h"
#include "element.h"
#include "table_metadata.h"
#include "element_allocator.h"
#include "file.h"
#include "table_data.h"

bool validate_row_schema(char *table_name,
                         struct TableField *first_table_field,
                         struct TableMetadataElement *table_metadata_element) {
    struct TableField *current_table_field = first_table_field;
    uint64_t current_table_field_index = 0;

    while (current_table_field_index != table_metadata_element->columns_count) {
        if (current_table_field == NULL) {
            logger(LL_ERROR, __func__, "Row has less fields than table %s", table_name);
            return false;
        }

        current_table_field = current_table_field->next;
        current_table_field_index++;
    }

    if (current_table_field != NULL) {
        logger(LL_ERROR, __func__, "Row has more fields than table %s", table_name);
        return false;
    }

    return true;
}

uint64_t calculate_table_data_field_size(struct TableField *first_table_field) {
    uint64_t table_data_field_size = 0;

    struct TableField *current_table_field = first_table_field;
    while (current_table_field != NULL) {
        table_data_field_size += sizeof(struct TableDataFieldElement) + current_table_field->size;
        current_table_field = current_table_field->next;
    }

    table_data_field_size += sizeof(struct TableDataFieldElement);

    return table_data_field_size;
}

void fill_table_data_field_element(void *file_data_pointer,
                                   uint64_t table_data_field_element_offset,
                                   struct TableField *first_table_field) {
    struct TableField *current_table_field = first_table_field;
    uint64_t current_table_data_field_value_offset =
        table_data_field_element_offset + ELEMENT_VALUE_OFFSET + TABLE_DATA_ELEMENT_FIRST_FIELD_OFFSET;
    struct TableDataFieldElement *current_table_data_field_element =
        (struct TableDataFieldElement *) ((char *) file_data_pointer + current_table_data_field_value_offset);

    while (current_table_field != NULL) {
        current_table_data_field_element->field_size = current_table_field->size;

        if (current_table_field->next != NULL) {
            current_table_data_field_element->next_field_offset =
                current_table_data_field_value_offset + current_table_field->size;
            current_table_data_field_element->has_next = true;
        } else {
            current_table_data_field_element->has_next = false;
        }

        memcpy(
            (char *) file_data_pointer + current_table_data_field_value_offset + sizeof(struct TableDataFieldElement),
            current_table_field->value,
            current_table_field->size);

        current_table_data_field_value_offset += current_table_field->size;
        current_table_data_field_element =
            (struct TableDataFieldElement *) ((char *) file_data_pointer + current_table_data_field_value_offset);
        current_table_field = current_table_field->next;
    }

    current_table_data_field_element->has_next = false;
}

int operation_insert(char *table_name, struct TableField *first_table_field) {
    logger(LL_INFO, __func__, "Inserting row into table %s", table_name);

    uint64_t table_metadata_offset;
    int find_table_metadata_offset_result = find_table_metadata_offset(table_name, &table_metadata_offset);
    if (find_table_metadata_offset_result == -1) {
        logger(LL_ERROR, __func__, "Cannot find table metadata about table %s", table_name);
        return -1;
    }

    void *file_data_pointer;
    int mmap_result = mmap_file(&file_data_pointer, 0, get_file_size());
    if (mmap_result == -1) {
        logger(LL_ERROR, __func__, "Cannot mmap file");
        return -1;
    }

    struct ElementHeader *element_header = (struct ElementHeader *) ((char *) file_data_pointer +
                                                                     table_metadata_offset);
    struct TableMetadataElement *table_metadata_element = (struct TableMetadataElement *) ((char *) element_header +
                                                                                           ELEMENT_VALUE_OFFSET);
    if (!validate_row_schema(table_name, first_table_field, table_metadata_element)) {
        return -1;
    }

    uint64_t table_data_field_size = calculate_table_data_field_size(first_table_field);
    uint64_t table_data_field_offset;
    int allocate_result = allocate_element(table_data_field_size, ET_TABLE_DATA,
                                           &table_data_field_offset);

    if (allocate_result == -1) {
        logger(LL_ERROR, __func__, "Cannot insert row into table %s because of allocation error", table_name);
        return -1;
    }

    fill_table_data_field_element(file_data_pointer, table_data_field_offset, first_table_field);

    struct TableDataElement *table_data_element = (struct TableDataElement *) ((char *) file_data_pointer +
                                                                               ELEMENT_VALUE_OFFSET +
                                                                               table_data_field_offset);
    if (table_metadata_element->has_rows) {
        table_data_element->has_prev_of_table = true;
        table_data_element->prev_of_table_offset = table_metadata_element->last_row_offset;

        table_metadata_element->last_row_offset = table_data_field_offset;
    } else {
        table_data_element->has_prev_of_table = false;

        table_metadata_element->last_row_offset = table_data_field_offset;
        table_metadata_element->has_rows = true;
    }

    return 0;
}

bool predicate_result_on_element(void *file_data_pointer,
                                 uint64_t table_data_element_offset,
                                 struct OperationPredicateParameter *parameters) {
    return true;
}

struct SelectResultIterator
operation_select(char *table_name, struct OperationPredicateParameter *parameters) {
    logger(LL_INFO, __func__, "Selecting rows from table %s", table_name);

    struct SelectResultIterator select_result_iterator;
    select_result_iterator.has_element = false;

    uint64_t table_metadata_offset;
    int find_table_metadata_offset_result = find_table_metadata_offset(table_name, &table_metadata_offset);
    if (find_table_metadata_offset_result == -1) {
        logger(LL_ERROR, __func__, "Cannot find table metadata about table %s", table_name);
        return select_result_iterator;
    }

    void *file_data_pointer;
    int mmap_result = mmap_file(&file_data_pointer, 0, get_file_size());
    if (mmap_result == -1) {
        logger(LL_ERROR, __func__, "Cannot mmap file");
        return select_result_iterator;
    }

    struct ElementHeader *element_header = (struct ElementHeader *) ((char *) file_data_pointer +
                                                                     table_metadata_offset);
    struct TableMetadataElement *table_metadata_element = (struct TableMetadataElement *) ((char *) element_header +
                                                                                           ELEMENT_VALUE_OFFSET);
    if (!table_metadata_element->has_rows) {
        logger(LL_INFO, __func__, "Table %s has no rows", table_name);
        return select_result_iterator;
    }

    uint64_t current_table_data_element_offset = table_metadata_element->last_row_offset;
    struct TableDataElement *current_table_data_element = (struct TableDataElement *) (
        (char *) file_data_pointer +
        ELEMENT_VALUE_OFFSET +
        current_table_data_element_offset);
    struct TableDataFieldElement *current_table_data_field_element = (struct TableDataFieldElement *) (
        (char *) file_data_pointer +
        current_table_data_element_offset +
        ELEMENT_VALUE_OFFSET +
        TABLE_DATA_ELEMENT_FIRST_FIELD_OFFSET);

    while (!predicate_result_on_element(file_data_pointer, current_table_data_element_offset, parameters)) {
        if (!current_table_data_element->has_prev_of_table) {
            logger(LL_INFO, __func__, "Cannot find row in table %s", table_name);
            return select_result_iterator;
        }

        current_table_data_element_offset = current_table_data_element->prev_of_table_offset;
        current_table_data_element = (struct TableDataElement *) ((char *) file_data_pointer +
                                                                  ELEMENT_VALUE_OFFSET +
                                                                  current_table_data_element_offset);
        current_table_data_field_element = (struct TableDataFieldElement *) ((char *) file_data_pointer +
                                                                             current_table_data_element_offset +
                                                                             ELEMENT_VALUE_OFFSET +
                                                                             TABLE_DATA_ELEMENT_FIRST_FIELD_OFFSET);
    }

    select_result_iterator.current_element_offset = current_table_data_element_offset;
    select_result_iterator.has_more = current_table_data_element->has_prev_of_table;
    select_result_iterator.has_element = true;

    return select_result_iterator;
}