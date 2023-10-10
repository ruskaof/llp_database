//
// Created by ruskaof on 1/10/23.
//

#include <string.h>
#include "../../include/data_operations.h"
#include "../utils/logging.h"
#include "element.h"
#include "table_metadata.h"
#include "element_allocator.h"
#include "file_private.h"
#include "table_data.h"

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
    void *file_data_pointer;
    int mmap_result = mmap_file(&file_data_pointer, 0, get_file_size());
    if (mmap_result == -1) {
        logger(LL_ERROR, __func__, "Cannot mmap file");
        return;
    }

    struct TableDataElement *table_data_element = (struct TableDataElement *) (
        (char *) file_data_pointer +
        ELEMENT_VALUE_OFFSET +
        table_data_element_offset);

    uint64_t current_table_data_field_element_offset = table_data_element_offset + ELEMENT_VALUE_OFFSET +
                                                       TABLE_DATA_ELEMENT_FIRST_FIELD_OFFSET;

    struct TableField *current_table_field = first_table_field;

    while (current_table_field != NULL) {
        struct TableDataFieldElement *table_data_field_element = (struct TableDataFieldElement *) (
            (char *) file_data_pointer +
            current_table_data_field_element_offset);
        table_data_field_element->has_next = current_table_field->next != NULL;
        table_data_field_element->field_size = current_table_field->size;
        if (table_data_field_element->has_next) {
            table_data_field_element->next_field_offset = current_table_data_field_element_offset +
                                                          sizeof(struct TableDataFieldElement) +
                                                          current_table_field->size;
        }

        memcpy((char *) file_data_pointer + current_table_data_field_element_offset +
               sizeof(struct TableDataFieldElement),
               current_table_field->value,
               current_table_field->size);

        current_table_data_field_element_offset = table_data_field_element->next_field_offset;
        current_table_field = current_table_field->next;
    }

    struct TableMetadataElement *table_metadata_element = (struct TableMetadataElement *) ((char *) file_data_pointer +
                                                                                           table_metadata_element_offset +
                                                                                           ELEMENT_VALUE_OFFSET);
    table_data_element->has_prev_of_table = table_metadata_element->has_rows;
    table_data_element->prev_of_table_offset = table_metadata_element->last_row_offset;

    table_metadata_element->has_rows = true;
    table_metadata_element->last_row_offset = table_data_element_offset;

    munmap_file(file_data_pointer, get_file_size());
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

bool parameter_check_equal(const void *file_data_pointer,
                           uint64_t value_to_cmp_offset,
                           const struct OperationPredicateParameter *current_parameter) {
    return memcmp(current_parameter->value,
                  (char *) file_data_pointer +
                  value_to_cmp_offset,
                  current_parameter->value_size) != 0;
}

bool parameter_check_not_equal(const void *file_data_pointer,
                               uint64_t value_to_cmp_offset,
                               const struct OperationPredicateParameter *current_parameter) {
    return !parameter_check_equal(file_data_pointer, value_to_cmp_offset, current_parameter);
}

bool parameter_check_greater_than(const void *file_data_pointer,
                                  uint64_t value_to_cmp_offset,
                                  const struct OperationPredicateParameter *current_parameter) {
    return memcmp(current_parameter->value,
                  (char *) file_data_pointer +
                  value_to_cmp_offset,
                  current_parameter->value_size) >= 0;
}

bool parameter_check_less_than(const void *file_data_pointer,
                               uint64_t value_to_cmp_offset,
                               const struct OperationPredicateParameter *current_parameter) {
    return memcmp(current_parameter->value,
                  (char *) file_data_pointer +
                  value_to_cmp_offset,
                  current_parameter->value_size) <= 0;
}

bool predicate_result_on_element(void *file_data_pointer,
                                 uint64_t table_data_element_offset,
                                 uint64_t table_metadata_element_offset,
                                 struct OperationPredicateParameter *parameters) {
    struct TableMetadataElement *table_metadata_element = (struct TableMetadataElement *) (
        (char *) file_data_pointer +
        table_metadata_element_offset +
        ELEMENT_VALUE_OFFSET);

    struct OperationPredicateParameter *current_parameter = parameters;

    while (current_parameter != NULL) {
        uint64_t current_table_data_field_element_offset = table_data_element_offset + ELEMENT_VALUE_OFFSET +
                                                           TABLE_DATA_ELEMENT_FIRST_FIELD_OFFSET;
        struct TableDataFieldElement *current_table_data_field_element = (struct TableDataFieldElement *) (
            (char *) file_data_pointer +
            table_data_element_offset +
            ELEMENT_VALUE_OFFSET +
            TABLE_DATA_ELEMENT_FIRST_FIELD_OFFSET);
        uint64_t current_column_index = 0;

        // find column for parameter
        while (strcmp(current_parameter->column_name, table_metadata_element->columns[current_column_index].name) !=
               0) {
            current_column_index++;
            current_table_data_field_element = (struct TableDataFieldElement *) (
                (char *) current_table_data_field_element +
                sizeof(struct TableDataFieldElement) +
                current_table_data_field_element->field_size);
            current_table_data_field_element_offset = current_table_data_field_element->next_field_offset;
        }

        if (current_parameter->value_size != current_table_data_field_element->field_size) {
            logger(LL_WARN, __func__, "Parameter value size is not equal to column value size: %ld != %ld",
                   current_parameter->value_size, current_table_data_field_element->field_size);
            return false;
        }

        uint64_t value_to_cmp_offset = current_table_data_field_element_offset + sizeof(struct TableDataFieldElement);

        switch (current_parameter->predicate_operator) {
            case PO_EQUAL:
                if (parameter_check_equal(file_data_pointer, value_to_cmp_offset, current_parameter)) {
                    return false;
                }
                break;
            case PO_NOT_EQUAL:
                if (parameter_check_not_equal(file_data_pointer, value_to_cmp_offset, current_parameter)) {
                    return false;
                }
                break;
            case PO_GREATER_THAN:
                if (parameter_check_greater_than(file_data_pointer, value_to_cmp_offset, current_parameter)) {
                    return false;
                }
                break;
            case PO_LESS_THAN:
                if (parameter_check_less_than(file_data_pointer, value_to_cmp_offset, current_parameter)) {
                    return false;
                }
                break;
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
    if (!table_metadata_element->has_rows) {
        logger(LL_INFO, __func__, "Table %s has no rows", table_name);
        return 0;
    }

    uint64_t current_table_data_element_offset = table_metadata_element->last_row_offset;
    struct TableDataElement *current_table_data_element = (struct TableDataElement *) (
        (char *) file_data_pointer +
        ELEMENT_VALUE_OFFSET +
        current_table_data_element_offset);

    while (current_table_data_element->has_prev_of_table) {
        logger(LL_DEBUG, __func__, "Deleting row at offset %ld", current_table_data_element_offset);
        uint64_t prev_of_table_offset = current_table_data_element->prev_of_table_offset;
        delete_element(current_table_data_element_offset);

        current_table_data_element_offset = prev_of_table_offset;
        current_table_data_element = (struct TableDataElement *) ((char *) file_data_pointer +
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

    void *file_data_pointer;
    int mmap_result = mmap_file(&file_data_pointer, 0, get_file_size());
    if (mmap_result == -1) {
        logger(LL_ERROR, __func__, "Cannot mmap file");
        return (struct SelectResultIterator) {.has_element = false, .has_more = false};
    }

    struct ElementHeader *element_header = (struct ElementHeader *) (
        (char *) file_data_pointer +
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
        (char *) file_data_pointer +
        ELEMENT_VALUE_OFFSET +
        current_table_data_element_offset);

    while (current_table_data_element->has_prev_of_table) {
        logger(LL_DEBUG, __func__, "Selecting row at offset %ld", current_table_data_element_offset);
        uint64_t prev_of_table_offset = current_table_data_element->prev_of_table_offset;

        if (predicate_result_on_element(file_data_pointer, current_table_data_element_offset, table_metadata_offset,
                                        parameters)) {
            return (struct SelectResultIterator) {.has_element = true, .has_more = true, .current_element_offset = current_table_data_element_offset};
        }

        current_table_data_element_offset = prev_of_table_offset;
        current_table_data_element = (struct TableDataElement *) ((char *) file_data_pointer +
                                                                  ELEMENT_VALUE_OFFSET +
                                                                  current_table_data_element_offset);
    }

    if (predicate_result_on_element(file_data_pointer, current_table_data_element_offset, table_metadata_offset,
                                    parameters)) {
        return (struct SelectResultIterator) {.has_element = true, .has_more = false, .current_element_offset = current_table_data_element_offset};
    }

    return (struct SelectResultIterator) {.has_element = false, .has_more = false};
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

    void *file_data_pointer;
    int mmap_result = mmap_file(&file_data_pointer, 0, get_file_size());
    if (mmap_result == -1) {
        logger(LL_ERROR, __func__, "Cannot mmap file");
        return (struct SelectResultIterator) {.has_element = false, .has_more = false};
    }

    struct TableDataElement *current_table_data_element = (struct TableDataElement *) ((char *) file_data_pointer +
                                                                                       ELEMENT_VALUE_OFFSET +
                                                                                       iterator->current_element_offset);

    if (!current_table_data_element->has_prev_of_table) {
        logger(LL_ERROR, __func__, "Cannot get next element from iterator without next element");
        return (struct SelectResultIterator) {.has_element = false, .has_more = false};
    }

    struct TableDataElement *prev_table_data_element = (struct TableDataElement *) ((char *) file_data_pointer +
                                                                                    ELEMENT_VALUE_OFFSET +
                                                                                    current_table_data_element->prev_of_table_offset);

    bool has_more_elements_result = prev_table_data_element->has_prev_of_table;
    uint64_t current_element_offset_result = current_table_data_element->prev_of_table_offset;

    int munmap_result = munmap_file(file_data_pointer, get_file_size());
    if (munmap_result == -1) {
        logger(LL_ERROR, __func__, "Cannot munmap file");
        return (struct SelectResultIterator) {.has_element = false, .has_more = false};
    }

    return (struct SelectResultIterator) {.has_element = true, .has_more = has_more_elements_result, .current_element_offset = current_element_offset_result};
}

struct TableField *get_by_iterator(struct SelectResultIterator *iterator) {
    if (!iterator->has_element) {
        logger(LL_ERROR, __func__, "Cannot get element from iterator without element");
        return NULL;
    }

    void *file_data_pointer;
    int mmap_result = mmap_file(&file_data_pointer, 0, get_file_size());
    if (mmap_result == -1) {
        logger(LL_ERROR, __func__, "Cannot mmap file");
        return NULL;
    }

    uint64_t current_table_data_field_element_offset = iterator->current_element_offset + ELEMENT_VALUE_OFFSET +
                                                       TABLE_DATA_ELEMENT_FIRST_FIELD_OFFSET;
    struct TableDataFieldElement *current_table_data_field_element = (struct TableDataFieldElement *) (
        (char *) file_data_pointer + current_table_data_field_element_offset);

    struct TableField *first_table_field = NULL;
    struct TableField *current_table_field = NULL;

    while (current_table_data_field_element->has_next) {
        struct TableField *table_field = malloc(sizeof(struct TableField));
        table_field->size = current_table_data_field_element->field_size;
        table_field->value = malloc(table_field->size);
        memcpy(table_field->value,
               (char *) file_data_pointer + current_table_data_field_element_offset +
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
        current_table_data_field_element = (struct TableDataFieldElement *) ((char *) file_data_pointer +
                                                                             current_table_data_field_element_offset);
    }

    struct TableField *table_field = malloc(sizeof(struct TableField));
    table_field->size = current_table_data_field_element->field_size;
    table_field->next = NULL;
    table_field->value = malloc(table_field->size);
    memcpy(table_field->value,
           (char *) file_data_pointer + current_table_data_field_element_offset +
           sizeof(struct TableDataFieldElement),
           table_field->size);

    if (first_table_field == NULL) {
        first_table_field = table_field;
    } else {
        current_table_field->next = table_field;
    }

    int munmap_result = munmap_file(file_data_pointer, get_file_size());
    if (munmap_result == -1) {
        logger(LL_ERROR, __func__, "Cannot munmap file");
        return NULL;
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

    void *file_data_pointer;
    int mmap_result = mmap_file(&file_data_pointer, 0, get_file_size());
    if (mmap_result == -1) {
        logger(LL_ERROR, __func__, "Cannot mmap file");
        return -1;
    }
    struct TableMetadataElement *table_metadata_element = (struct TableMetadataElement *) (
        (char *) file_data_pointer +
        table_metadata_offset +
        ELEMENT_VALUE_OFFSET);
    uint64_t current_table_data_element_offset = table_metadata_element->last_row_offset;

    struct TableDataElement *current_table_data_element = (struct TableDataElement *) (
        (char *) file_data_pointer +
        ELEMENT_VALUE_OFFSET +
        current_table_data_element_offset);

    if (predicate_result_on_element(file_data_pointer, current_table_data_element_offset, table_metadata_offset,
                                    parameters)) {
        logger(LL_DEBUG, __func__, "Deleting row at offset %ld", current_table_data_element_offset);
        table_metadata_element->has_rows = current_table_data_element->has_prev_of_table;
        table_metadata_element->last_row_offset = current_table_data_element->prev_of_table_offset;
        delete_element(current_table_data_element_offset);
    }

    struct TableDataElement *previous_table_data_element = current_table_data_element;
    current_table_data_element_offset = previous_table_data_element->prev_of_table_offset;
    current_table_data_element = (struct TableDataElement *) (
        (char *) file_data_pointer +
        ELEMENT_VALUE_OFFSET +
        current_table_data_element_offset);

    while (current_table_data_element->has_prev_of_table) {
        logger(LL_DEBUG, __func__, "Deleting row at offset %ld", current_table_data_element_offset);

        if (predicate_result_on_element(file_data_pointer, current_table_data_element_offset, table_metadata_offset,
                                        parameters)) {
            previous_table_data_element->has_prev_of_table = current_table_data_element->has_prev_of_table;
            previous_table_data_element->prev_of_table_offset = current_table_data_element->prev_of_table_offset;
            delete_element(current_table_data_element_offset);
        } else {
            previous_table_data_element = current_table_data_element;
        }

        current_table_data_element_offset = previous_table_data_element->prev_of_table_offset;
        current_table_data_element = (struct TableDataElement *) (
            (char *) file_data_pointer +
            ELEMENT_VALUE_OFFSET +
            current_table_data_element_offset);
    }

    if (predicate_result_on_element(file_data_pointer, current_table_data_element_offset, table_metadata_offset,
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
