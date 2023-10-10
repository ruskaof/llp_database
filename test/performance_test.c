//
// Created by ruskaof on 10/10/23.
//

#include "../include/data_operations.h"
#include "../include/table_operations.h"
#include "../src/db/file.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#if defined(__linux__)

#define TEST_FILE_LOCATION "/home/ruskaof/Desktop/testdb2"

#endif

#if defined(_WIN32)

#define TEST_FILE_LOCATION "C:\\Users\\199-4\\Desktop\\testdb2"

#endif

void insertion_performance_tests(uint32_t n) {
    init_db(TEST_FILE_LOCATION);

    char table_name[] = "test_table";
    struct TableColumn columns[] = {
        {.name="id", .type=TD_INT64},
        {.name="name", .type=TD_STRING},
        {.name="age", .type=TD_INT64},
        {.name="is_active", .type=TD_BOOL},
        {.name="height", .type=TD_FLOAT64},
    };
    uint64_t columns_count = 5;

    int create_table_result = operation_create_table(table_name, columns, columns_count);
    if (create_table_result == -1) {
        printf("Cannot create table\n");
        return;
    }

    for (uint32_t i = 0; i < n; i++) {
        struct TableField *fifth_table_field = malloc(sizeof(struct TableField));
        fifth_table_field->size = sizeof(double);
        fifth_table_field->value = malloc(sizeof(double));
        *(double *) fifth_table_field->value = i + 0.5;
        fifth_table_field->next = NULL;

        struct TableField *fourth_table_field = malloc(sizeof(struct TableField));
        fourth_table_field->size = sizeof(bool);
        fourth_table_field->value = malloc(sizeof(bool));
        *(bool *) fourth_table_field->value = i % 2 == 0;
        fourth_table_field->next = fifth_table_field;

        struct TableField *third_table_field = malloc(sizeof(struct TableField));
        third_table_field->size = sizeof(int64_t);
        third_table_field->value = malloc(sizeof(int64_t));
        *(int64_t *) third_table_field->value = i;
        third_table_field->next = fourth_table_field;

        struct TableField *second_table_field = malloc(sizeof(struct TableField));
        second_table_field->size = 1024 * 1024; // 1 MB
        second_table_field->value = malloc(second_table_field->size);
        memset(second_table_field->value, 'a', second_table_field->size);
        second_table_field->next = third_table_field;

        struct TableField *first_table_field = malloc(sizeof(struct TableField));
        first_table_field->size = sizeof(int64_t);
        first_table_field->value = malloc(sizeof(int64_t));
        *(int64_t *) first_table_field->value = i;
        first_table_field->next = second_table_field;

        int insert_result = operation_insert(table_name, first_table_field);
        if (insert_result == -1) {
            printf("Cannot insert row\n");
            return;
        }

        free(first_table_field->value);
        free(first_table_field);
        free(second_table_field->value);
        free(second_table_field);
        free(third_table_field->value);
        free(third_table_field);
        free(fourth_table_field->value);
        free(fourth_table_field);
        free(fifth_table_field->value);
        free(fifth_table_field);
    }

    printf("\033[0;32m");
    printf("Insertion of %d rows was successful.\n", n);
    printf("\033[0m");

    close_db();
    delete_db_file(TEST_FILE_LOCATION);
}

int main(int argc, char **argv) {
    if (argc != 3) {
        printf("Usage: %s <test_name> <test-arg>\n", argv[0]);
        return 1;
    }

    if (strcmp(argv[1], "insertion") == 0) {
        insertion_performance_tests(atoi(argv[2]));

        return 0;
    } else {
        printf("Unknown test name: %s\n", argv[1]);
        return 1;
    }
}