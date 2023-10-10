//
// Created by ruskaof on 10/10/23.
//

#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include "../include/table.h"
#include "../include/table_operations.h"
#include "../include/data_operations.h"
#include "../include/db.h"

#if defined(__linux__) || defined(__APPLE__)

#define TEST_FILE_LOCATION "/home/ruskaof/Desktop/testdb2"

#endif

#if defined(_WIN32)

#define TEST_FILE_LOCATION "C:\\Users\\199-4\\Desktop\\testdb2"

#endif

void print_table_row(struct TableField *table_field) {
    struct TableField *current_table_field = table_field;
    while (current_table_field != NULL) {
        if (current_table_field->size == sizeof(int64_t)) {
            printf("%ld ", *(int64_t *) current_table_field->value);
        } else if (current_table_field->size == sizeof(char) * 5) {
            printf("%s ", (char *) current_table_field->value);
        }
        current_table_field = current_table_field->next;
    }
    printf("\n");
}

void join_test() {
    init_db(TEST_FILE_LOCATION);

    TableSchemaName table1_name = "student";
    struct TableColumn table1_columns[] = {
        {.name = "id", .type = TD_INT64},
        {.name = "age", .type = TD_INT64},
        {.name = "group_id", .type = TD_INT64},
    };
    uint64_t table1_columns_n = sizeof(table1_columns) / sizeof(struct TableColumn);

    operation_create_table(table1_name, table1_columns, table1_columns_n);

    TableSchemaName table2_name = "group";
    struct TableColumn table2_columns[] = {
        {.name = "id", .type = TD_INT64},
        {.name = "name", .type = TD_STRING},
    };
    uint64_t table2_columns_n = sizeof(table2_columns) / sizeof(struct TableColumn);

    operation_create_table(table2_name, table2_columns, table2_columns_n);

    struct TableField *table1_row1 = create_table_row(sizeof(int64_t), &(int64_t) {1},
                                                      sizeof(int64_t), &(int64_t) {20},
                                                      sizeof(int64_t), &(int64_t) {1}, 0);
    struct TableField *table1_row2 = create_table_row(sizeof(int64_t), &(int64_t) {2},
                                                      sizeof(int64_t), &(int64_t) {15},
                                                      sizeof(int64_t), &(int64_t) {2}, 0);
    struct TableField *table1_row3 = create_table_row(sizeof(int64_t),
                                                      &(int64_t) {3},
                                                      sizeof(int64_t), &(int64_t) {19},
                                                      sizeof(int64_t), &(int64_t) {2}, 0);
    struct TableField *table1_row4 = create_table_row(sizeof(int64_t), &(int64_t) {4},
                                                      sizeof(int64_t), &(int64_t) {15},
                                                      sizeof(int64_t), &(int64_t) {2}, 0);
    struct TableField *table1_row5 = create_table_row(sizeof(int64_t), &(int64_t) {5},
                                                      sizeof(int64_t), &(int64_t) {16},
                                                      sizeof(int64_t), &(int64_t) {6}, 0);
    struct TableField *table1_row6 = create_table_row(sizeof(int64_t),
                                                      &(int64_t) {1},
                                                      sizeof(int64_t), &(int64_t) {18},
                                                      sizeof(int64_t), &(int64_t) {1}, 0);
    struct TableField *table1_row7 = create_table_row(sizeof(int64_t), &(int64_t) {2},
                                                      sizeof(int64_t), &(int64_t) {17},
                                                      sizeof(int64_t), &(int64_t) {1}, 0);

    operation_insert(table1_name, table1_row1);
    operation_insert(table1_name, table1_row2);
    operation_insert(table1_name, table1_row3);
    operation_insert(table1_name, table1_row4);
    operation_insert(table1_name, table1_row5);
    operation_insert(table1_name, table1_row6);
    operation_insert(table1_name, table1_row7);

    free_table_row_without_values(table1_row1);
    free_table_row_without_values(table1_row2);
    free_table_row_without_values(table1_row3);
    free_table_row_without_values(table1_row4);
    free_table_row_without_values(table1_row5);
    free_table_row_without_values(table1_row6);
    free_table_row_without_values(table1_row7);

    struct TableField *table2_row1 = create_table_row(sizeof(int64_t), &(int64_t) {1},
                                                      sizeof(char) * 6, "P33301", 0);
    struct TableField *table2_row2 = create_table_row(sizeof(int64_t), &(int64_t) {2},
                                                      sizeof(char) * 6, "P33302", 0);
    struct TableField *table2_row3 = create_table_row(sizeof(int64_t), &(int64_t) {3},
                                                      sizeof(char) * 6, "P33303", 0);

    operation_insert(table2_name, table2_row1);
    operation_insert(table2_name, table2_row2);
    operation_insert(table2_name, table2_row3);

    free_table_row_without_values(table2_row1);
    free_table_row_without_values(table2_row2);
    free_table_row_without_values(table2_row3);

    struct OperationPredicateParameter first_table_age_greater_than_19 = {
        .column_name = "age",
        .value_size = sizeof(int64_t),
        .value = &(int64_t) {18},
        .next = NULL,
        .predicate_operator = PO_GREATER_THAN,
    };

    operation_inner_join("student", &first_table_age_greater_than_19, "group_id", "group", NULL, "id", "student_group");

    struct SelectResultIterator select_result_iterator = operation_select("student_group", NULL);

    while (select_result_iterator.has_element) {
        struct TableField *table_field = get_by_iterator(&select_result_iterator);
        struct TableField *current_table_field = table_field;
        while (current_table_field != NULL) {
            if (current_table_field->size == sizeof(int64_t)) {
                printf("%ld ", *(int64_t *) current_table_field->value);
            } else if (current_table_field->size == sizeof(char) * 6) {
                char *value = malloc(sizeof(char) * 7);
                memcpy(value, current_table_field->value, sizeof(char) * 6);
                value[6] = '\0';
                printf("%s ", value);
            }
            current_table_field = current_table_field->next;
        }
        printf("\n");
        select_result_iterator = get_next(&select_result_iterator);

        free_table_row(table_field);
    }

    close_db();
    delete_db_file(TEST_FILE_LOCATION);
}

int main() {
    join_test();
}