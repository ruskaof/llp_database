//
// Created by ruskaof on 10/10/23.
//

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include "../../include/table.h"

void free_table_row(struct TableField *table_field) {
    if (table_field->next != NULL) {
        free_table_row(table_field->next);
    }
    free(table_field->value);
    free(table_field);
}

void free_table_row_without_values(struct TableField *table_field) {
    if (table_field->next != NULL) {
        free_table_row_without_values(table_field->next);
    }
    free(table_field);
}

struct TableField *create_table_row(uint64_t first_field_size, void *first_field_value, ...) {
    struct TableField *first_table_field = malloc(sizeof(struct TableField));
    first_table_field->size = first_field_size;
    first_table_field->value = first_field_value;

    struct TableField *current_table_field = first_table_field;

    va_list args;
    va_start(args, first_field_value);

    while (true) {
        uint64_t field_size = va_arg(args, uint64_t);
        if (field_size == 0) {
            break;
        }

        void *field_value = va_arg(args, void *);

        struct TableField *next_table_field = malloc(sizeof(struct TableField));
        next_table_field->size = field_size;
        next_table_field->value = field_value;
        next_table_field->next = NULL;

        current_table_field->next = next_table_field;
        current_table_field = next_table_field;
    }

    return first_table_field;
}
