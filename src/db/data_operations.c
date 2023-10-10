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
#include "comparator.h"

uint64_t calculate_table_data_size(struct TableField *first_table_field) {
    uint64_t table_data_field_size = 0;

    struct TableField *current_table_field = first_table_field;
    while (current_table_field != NULL) {
        table_data_field_size += sizeof(struct TableDataFieldElement) + current_table_field->size;
        current_table_field = current_table_field->next;
    }

    table_data_field_size += sizeof(struct TableDataFieldElement);

    return table_data_field_size;
}

void fill_newly_allocated_element_with_data(struct TableField *first_table_field,
                                            uint64_t table_data_element_offset,
                                            uint64_t table_metadata_element_offset) {

    struct TableDataElement *table_data_element = (struct TableDataElement *) (
        (char *) get_file_data_pointer() +
        ELEMENT_VALUE_OFFSET +
        table_data_element_offset);

    uint64_t current_table_data_field_element_offset = table_data_element_offset + ELEMENT_VALUE_OFFSET +
                                                       TABLE_DATA_ELEMENT_FIRST_FIELD_OFFSET;

    struct TableField *current_table_field = first_table_field;

    while (current_table_field != NULL) {
        struct TableDataFieldElement *table_data_field_element = (struct TableDataFieldElement *) (
            (char *) get_file_data_pointer() +
            current_table_data_field_element_offset);
        table_data_field_element->has_next = current_table_field->next != NULL;
        table_data_field_element->field_size = current_table_field->size;
        if (table_data_field_element->has_next) {
            table_data_field_element->next_field_offset = current_table_data_field_element_offset +
                                                          sizeof(struct TableDataFieldElement) +
                                                          current_table_field->size;
        }

        memcpy((char *) get_file_data_pointer() + current_table_data_field_element_offset +
               sizeof(struct TableDataFieldElement),
               current_table_field->value,
               current_table_field->size);

        current_table_data_field_element_offset = table_data_field_element->next_field_offset;
        current_table_field = current_table_field->next;
    }

    struct TableMetadataElement *table_metadata_element = (struct TableMetadataElement *) (
        (char *) get_file_data_pointer() +
        table_metadata_element_offset +
        ELEMENT_VALUE_OFFSET);
    table_data_element->has_prev_of_table = table_metadata_element->has_rows;
    table_data_element->prev_of_table_offset = table_metadata_element->last_row_offset;

    table_metadata_element->has_rows = true;
    table_metadata_element->last_row_offset = table_data_element_offset;
}

int operation_insert(char *table_name, struct TableField *first_table_field) {
    logger(LL_INFO, __func__, "Inserting row into table %s", table_name);

    uint64_t table_metadata_offset;
    int find_table_metadata_offset_result = find_table_metadata_offset(table_name, &table_metadata_offset);
    if (find_table_metadata_offset_result == -1) {
        logger(LL_ERROR, __func__, "Cannot find table metadata about table %s", table_name);
        return -1;
    }

    uint64_t table_data_element_size = calculate_table_data_size(first_table_field);
    uint64_t table_data_element_offset;
    int allocate_result = allocate_element(table_data_element_size, ET_TABLE_DATA,
                                           &table_data_element_offset);
    if (allocate_result == -1) {
        logger(LL_ERROR, __func__, "Cannot insert row into table %s because of allocation error", table_name);
        return -1;
    }

    fill_newly_allocated_element_with_data(first_table_field, table_data_element_offset, table_metadata_offset);
    return 0;
}

bool predicate_result_on_element(uint64_t table_data_element_offset,
                                 uint64_t table_metadata_element_offset,
                                 struct OperationPredicateParameter *parameters) {
    struct TableMetadataElement *table_metadata_element = (struct TableMetadataElement *) (
        (char *) get_file_data_pointer() +
        table_metadata_element_offset +
        ELEMENT_VALUE_OFFSET);

    struct OperationPredicateParameter *current_parameter = parameters;

    while (current_parameter != NULL) {
        uint64_t current_table_data_field_element_offset = table_data_element_offset + ELEMENT_VALUE_OFFSET +
                                                           TABLE_DATA_ELEMENT_FIRST_FIELD_OFFSET;
        struct TableDataFieldElement *current_table_data_field_element = (struct TableDataFieldElement *) (
            (char *) get_file_data_pointer() +
            table_data_element_offset +
            ELEMENT_VALUE_OFFSET +
            TABLE_DATA_ELEMENT_FIRST_FIELD_OFFSET);
        uint64_t current_column_index = 0;

        // find column for parameter
        while (memcmp(current_parameter->column_name,
                      table_metadata_element->columns[current_column_index].name,
                      MAX_TABLE_COLUMN_NAME_LENGTH) != 0) {
            logger(LL_DEBUG, __func__, "Comparing column name %s with %s",
                   current_parameter->column_name,
                   table_metadata_element->columns[current_column_index].name);

            if (!current_table_data_field_element->has_next) {
                logger(LL_WARN, __func__, "Cannot find column with name %s",
                       current_parameter->column_name);
                return false;
            }

            current_column_index++;
            current_table_data_field_element_offset = current_table_data_field_element->next_field_offset;
            current_table_data_field_element = (struct TableDataFieldElement *) (
                (char *) get_file_data_pointer() +
                current_table_data_field_element_offset);
        }

        enum TableDatatype values_types = table_metadata_element->columns[current_column_index].type;
        uint64_t parameter_value_size = current_parameter->value_size;
        uint64_t column_value_size = current_table_data_field_element->field_size;
        void *parameter_value = current_parameter->value;
        void *column_value = (char *) get_file_data_pointer() + current_table_data_field_element_offset +
                             sizeof(struct TableDataFieldElement);

        switch (current_parameter->predicate_operator) {
            case PO_EQUAL:
                return first_value_is_equal_to_second(values_types, parameter_value_size, column_value_size,
                                                      parameter_value, column_value);
            case PO_NOT_EQUAL:
                return !first_value_is_equal_to_second(values_types, parameter_value_size, column_value_size,
                                                       parameter_value, column_value);
            case PO_GREATER_THAN:
                return first_value_is_greater_than_second(values_types, parameter_value_size, column_value_size,
                                                          parameter_value, column_value);
            case PO_LESS_THAN:
                return first_value_is_less_than_second(values_types, parameter_value_size, column_value_size,
                                                       parameter_value, column_value);
        }

        current_parameter = current_parameter->next;
    }

    logger(LL_INFO, __func__, "Row at offset %ld matches predicate", table_data_element_offset);
    return true;
}

int operation_truncate(char *table_name) {
    logger(LL_INFO, __func__, "Truncating table %s", table_name);

    uint64_t table_metadata_offset;
    int find_table_metadata_offset_result = find_table_metadata_offset(table_name, &table_metadata_offset);
    if (find_table_metadata_offset_result == -1) {
        logger(LL_ERROR, __func__, "Cannot find table metadata about table %s", table_name);
        return -1;
    }

    struct ElementHeader *element_header = (struct ElementHeader *) ((char *) get_file_data_pointer() +
                                                                     table_metadata_offset);
    struct TableMetadataElement *table_metadata_element = (struct TableMetadataElement *) ((char *) element_header +
                                                                                           ELEMENT_VALUE_OFFSET);
    if (!table_metadata_element->has_rows) {
        logger(LL_INFO, __func__, "Table %s has no rows", table_name);
        return 0;
    }

    uint64_t current_table_data_element_offset = table_metadata_element->last_row_offset;
    struct TableDataElement *current_table_data_element = (struct TableDataElement *) (
        (char *) get_file_data_pointer() +
        ELEMENT_VALUE_OFFSET +
        current_table_data_element_offset);

    while (current_table_data_element->has_prev_of_table) {
        logger(LL_DEBUG, __func__, "Deleting row at offset %ld", current_table_data_element_offset);
        uint64_t prev_of_table_offset = current_table_data_element->prev_of_table_offset;
        delete_element(current_table_data_element_offset);

        current_table_data_element_offset = prev_of_table_offset;
        current_table_data_element = (struct TableDataElement *) ((char *) get_file_data_pointer() +
                                                                  ELEMENT_VALUE_OFFSET +
                                                                  current_table_data_element_offset);
    }

    delete_element(current_table_data_element_offset);

    table_metadata_element->has_rows = false;

    return 0;
}

struct SelectResultIterator
operation_select(char *table_name, struct OperationPredicateParameter *parameters) {
    logger(LL_INFO, __func__, "Selecting rows from table %s", table_name);

    uint64_t table_metadata_offset;
    int find_table_metadata_offset_result = find_table_metadata_offset(table_name, &table_metadata_offset);
    if (find_table_metadata_offset_result == -1) {
        logger(LL_ERROR, __func__, "Cannot find table metadata about table %s", table_name);
        return (struct SelectResultIterator) {.has_element = false, .has_more = false};
    }

    struct ElementHeader *element_header = (struct ElementHeader *) (
        (char *) get_file_data_pointer() +
        table_metadata_offset);
    struct TableMetadataElement *table_metadata_element = (struct TableMetadataElement *) (
        (char *) element_header +
        ELEMENT_VALUE_OFFSET);

    if (!table_metadata_element->has_rows) {
        logger(LL_INFO, __func__, "Table %s has no rows", table_name);
        return (struct SelectResultIterator) {.has_element = false, .has_more = false};
    }

    uint64_t current_table_data_element_offset = table_metadata_element->last_row_offset;
    struct TableDataElement *current_table_data_element = (struct TableDataElement *) (
        (char *) get_file_data_pointer() +
        ELEMENT_VALUE_OFFSET +
        current_table_data_element_offset);

    while (current_table_data_element->has_prev_of_table) {
        logger(LL_DEBUG, __func__, "Selecting row at offset %ld", current_table_data_element_offset);
        uint64_t prev_of_table_offset = current_table_data_element->prev_of_table_offset;

        if (predicate_result_on_element(current_table_data_element_offset, table_metadata_offset,
                                        parameters)) {
            return (struct SelectResultIterator) {.has_element = true, .has_more = true, .current_element_offset = current_table_data_element_offset, .parameters = parameters, .table_metadata_offset=table_metadata_offset};
        }

        current_table_data_element_offset = prev_of_table_offset;
        current_table_data_element = (struct TableDataElement *) ((char *) get_file_data_pointer() +
                                                                  ELEMENT_VALUE_OFFSET +
                                                                  current_table_data_element_offset);
    }

    if (predicate_result_on_element(current_table_data_element_offset, table_metadata_offset,
                                    parameters)) {
        return (struct SelectResultIterator) {.has_element = true, .has_more = false, .current_element_offset = current_table_data_element_offset, .parameters = parameters, .table_metadata_offset=table_metadata_offset};
    }

    return (struct SelectResultIterator) {.has_element = false, .has_more = false, .parameters = parameters, .table_metadata_offset=table_metadata_offset};
}

struct SelectResultIterator get_next(struct SelectResultIterator *iterator) {
    logger(LL_INFO, __func__, "Getting next element from iterator with current offset %ld",
           iterator->current_element_offset);
    if (!iterator->has_element) {
        logger(LL_ERROR, __func__, "Cannot get next element from iterator without element");
        return (struct SelectResultIterator) {.has_element = false, .has_more = false};
    }

    if (!iterator->has_more) {
        logger(LL_ERROR, __func__, "Cannot get next element from iterator without more elements");
        return (struct SelectResultIterator) {.has_element = false, .has_more = false};
    }

    struct TableDataElement *current_table_data_element = (struct TableDataElement *) (
        (char *) get_file_data_pointer() +
        ELEMENT_VALUE_OFFSET +
        iterator->current_element_offset);

    if (!current_table_data_element->has_prev_of_table) {
        logger(LL_ERROR, __func__, "Cannot get next element from iterator without next element");
        return (struct SelectResultIterator) {.has_element = false, .has_more = false};
    }

    while (current_table_data_element->has_prev_of_table) {
        struct TableDataElement *prev_table_data_element = (struct TableDataElement *) (
            (char *) get_file_data_pointer() +
            ELEMENT_VALUE_OFFSET +
            current_table_data_element->prev_of_table_offset);

        if (predicate_result_on_element(current_table_data_element->prev_of_table_offset,
                                        iterator->table_metadata_offset,
                                        iterator->parameters)) {
            return (struct SelectResultIterator) {.has_element = true,
                .has_more = prev_table_data_element->has_prev_of_table,
                .current_element_offset = current_table_data_element->prev_of_table_offset,
                .table_metadata_offset = iterator->table_metadata_offset,
                .parameters = iterator->parameters};
        }

        current_table_data_element = prev_table_data_element;
    }

    return (struct SelectResultIterator) {.has_element = false, .has_more = false};
}

struct TableField *get_by_iterator(struct SelectResultIterator *iterator) {
    if (!iterator->has_element) {
        logger(LL_DEBUG, __func__, "Cannot get element from iterator without element");
        return NULL;
    }

    uint64_t current_table_data_field_element_offset = iterator->current_element_offset + ELEMENT_VALUE_OFFSET +
                                                       TABLE_DATA_ELEMENT_FIRST_FIELD_OFFSET;
    struct TableDataFieldElement *current_table_data_field_element = (struct TableDataFieldElement *) (
        (char *) get_file_data_pointer() + current_table_data_field_element_offset);

    struct TableField *first_table_field = NULL;
    struct TableField *current_table_field = NULL;

    while (current_table_data_field_element->has_next) {
        struct TableField *table_field = malloc(sizeof(struct TableField));
        table_field->size = current_table_data_field_element->field_size;
        table_field->value = malloc(table_field->size);
        memcpy(table_field->value,
               (char *) get_file_data_pointer() + current_table_data_field_element_offset +
               sizeof(struct TableDataFieldElement),
               table_field->size);

        if (first_table_field == NULL) {
            first_table_field = table_field;
            current_table_field = table_field;
        } else {
            current_table_field->next = table_field;
            current_table_field = table_field;
        }

        current_table_data_field_element_offset = current_table_data_field_element->next_field_offset;
        current_table_data_field_element = (struct TableDataFieldElement *) ((char *) get_file_data_pointer() +
                                                                             current_table_data_field_element_offset);
    }

    struct TableField *table_field = malloc(sizeof(struct TableField));
    table_field->size = current_table_data_field_element->field_size;
    table_field->next = NULL;
    table_field->value = malloc(table_field->size);
    memcpy(table_field->value,
           (char *) get_file_data_pointer() + current_table_data_field_element_offset +
           sizeof(struct TableDataFieldElement),
           table_field->size);

    if (first_table_field == NULL) {
        first_table_field = table_field;
    } else {
        current_table_field->next = table_field;
    }

    return first_table_field;
}

int operation_delete(char *table_name, struct OperationPredicateParameter *parameters) {
    logger(LL_INFO, __func__, "Deleting rows from table %s", table_name);

    uint64_t table_metadata_offset;
    int find_table_metadata_offset_result = find_table_metadata_offset(table_name, &table_metadata_offset);
    if (find_table_metadata_offset_result == -1) {
        logger(LL_ERROR, __func__, "Cannot find table metadata about table %s", table_name);
        return -1;
    }

    struct TableMetadataElement *table_metadata_element = (struct TableMetadataElement *) (
        (char *) get_file_data_pointer() +
        table_metadata_offset +
        ELEMENT_VALUE_OFFSET);
    uint64_t current_table_data_element_offset = table_metadata_element->last_row_offset;

    struct TableDataElement *current_table_data_element = (struct TableDataElement *) (
        (char *) get_file_data_pointer() +
        ELEMENT_VALUE_OFFSET +
        current_table_data_element_offset);

    if (predicate_result_on_element(current_table_data_element_offset, table_metadata_offset,
                                    parameters)) {
        logger(LL_DEBUG, __func__, "Deleting row at offset %ld", current_table_data_element_offset);
        table_metadata_element->has_rows = current_table_data_element->has_prev_of_table;
        table_metadata_element->last_row_offset = current_table_data_element->prev_of_table_offset;
        delete_element(current_table_data_element_offset);
    }

    struct TableDataElement *previous_table_data_element = current_table_data_element;
    current_table_data_element_offset = previous_table_data_element->prev_of_table_offset;
    current_table_data_element = (struct TableDataElement *) (
        (char *) get_file_data_pointer() +
        ELEMENT_VALUE_OFFSET +
        current_table_data_element_offset);

    while (current_table_data_element->has_prev_of_table) {

        if (predicate_result_on_element(current_table_data_element_offset, table_metadata_offset,
                                        parameters)) {
            logger(LL_DEBUG, __func__, "Deleting row at offset %ld", current_table_data_element_offset);
            previous_table_data_element->has_prev_of_table = current_table_data_element->has_prev_of_table;
            previous_table_data_element->prev_of_table_offset = current_table_data_element->prev_of_table_offset;
            delete_element(current_table_data_element_offset);
        } else {
            previous_table_data_element = current_table_data_element;
        }

        current_table_data_element_offset = previous_table_data_element->prev_of_table_offset;
        current_table_data_element = (struct TableDataElement *) (
            (char *) get_file_data_pointer() +
            ELEMENT_VALUE_OFFSET +
            current_table_data_element_offset);
    }

    if (predicate_result_on_element(current_table_data_element_offset, table_metadata_offset,
                                    parameters)) {
        previous_table_data_element->has_prev_of_table = false;
        delete_element(current_table_data_element_offset);
    } else {
        previous_table_data_element->has_prev_of_table = true;
        previous_table_data_element->prev_of_table_offset = current_table_data_element_offset;
    }

    return 0;
}

int operation_update(char *table_name,
                     struct OperationPredicateParameter *parameters,
                     struct TableField *first_table_field) {
    logger(LL_INFO, __func__, "Updating rows from table %s", table_name);

    int delete_result = operation_delete(table_name, parameters);
    if (delete_result == -1) {
        logger(LL_ERROR, __func__, "Cannot update rows from table %s because of deletion error", table_name);
        return -1;
    }

    int insert_result = operation_insert(table_name, first_table_field);
    if (insert_result == -1) {
        logger(LL_ERROR, __func__, "Cannot update rows from table %s because of insertion error", table_name);
        return -1;
    }

    return 0;
}
