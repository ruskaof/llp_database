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

#include <sys/time.h>

uint64_t get_current_time_millis() {
    struct timeval te;
    gettimeofday(&te, NULL);
    uint64_t milliseconds = te.tv_sec * 1000LL + te.tv_usec / 1000;
    return milliseconds;
}

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
        uint64_t time_mills = get_current_time_millis();

        for (uint32_t j = 0; j < i; j++) {
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

        printf("Insertion of %d rows was successful. Time: %ld\n", i, get_current_time_millis() - time_mills);
        operation_truncate(table_name);
    }

    printf("Insertion of %d rows was successful.\n", n);

    close_db();
    delete_db_file(TEST_FILE_LOCATION);
}

void update_performance_tests(uint32_t n) {
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
    }

    struct TableField *fifth_table_field = malloc(sizeof(struct TableField));
    fifth_table_field->size = sizeof(double);
    fifth_table_field->value = malloc(sizeof(double));
    *(double *) fifth_table_field->value = 1 + 0.5;
    fifth_table_field->next = NULL;

    struct TableField *fourth_table_field = malloc(sizeof(struct TableField));
    fourth_table_field->size = sizeof(bool);
    fourth_table_field->value = malloc(sizeof(bool));
    *(bool *) fourth_table_field->value = 0;
    fourth_table_field->next = fifth_table_field;

    struct TableField *third_table_field = malloc(sizeof(struct TableField));
    third_table_field->size = sizeof(int64_t);
    third_table_field->value = malloc(sizeof(int64_t));
    *(int64_t *) third_table_field->value = 1;
    third_table_field->next = fourth_table_field;

    struct TableField *second_table_field = malloc(sizeof(struct TableField));
    second_table_field->size = 1024 * 1024; // 1 MB
    second_table_field->value = malloc(second_table_field->size);
    memset(second_table_field->value, 'a', second_table_field->size);
    second_table_field->next = third_table_field;

    struct TableField *first_table_field = malloc(sizeof(struct TableField));
    first_table_field->size = sizeof(int64_t);
    first_table_field->value = malloc(sizeof(int64_t));
    *(int64_t *) first_table_field->value = 1;
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

    // update
    for (uint64_t i = 0; i < n; i++) {
        struct OperationPredicateParameter parameter = {
            .next = NULL,
            .column_name = "id",
            .predicate_operator = PO_EQUAL,
            .value_size = sizeof(int64_t),
            .value = &(int64_t) {1},
        };

        fifth_table_field = malloc(sizeof(struct TableField));
        fifth_table_field->size = sizeof(double);
        fifth_table_field->value = malloc(sizeof(double));
        *(double *) fifth_table_field->value = i + 0.5;
        fifth_table_field->next = NULL;

        fourth_table_field = malloc(sizeof(struct TableField));
        fourth_table_field->size = sizeof(bool);
        fourth_table_field->value = malloc(sizeof(bool));
        *(bool *) fourth_table_field->value = i % 2 == 0;
        fourth_table_field->next = fifth_table_field;

        third_table_field = malloc(sizeof(struct TableField));
        third_table_field->size = sizeof(int64_t);
        third_table_field->value = malloc(sizeof(int64_t));
        *(int64_t *) third_table_field->value = i;
        third_table_field->next = fourth_table_field;

        second_table_field = malloc(sizeof(struct TableField));
        second_table_field->size = 1024 * 1024; // 1 MB
        second_table_field->value = malloc(second_table_field->size);
        memset(second_table_field->value, 'a', second_table_field->size);
        second_table_field->next = third_table_field;

        first_table_field = malloc(sizeof(struct TableField));
        first_table_field->size = sizeof(int64_t);
        first_table_field->value = malloc(sizeof(int64_t));
        *(int64_t *) first_table_field->value = 1;
        first_table_field->next = second_table_field;

        int update_result = operation_update(table_name, &parameter, first_table_field);

        if (update_result == -1) {
            printf("Cannot update row\n");
            return;
        }
    }


    printf("Update of %d rows was successful.\n", n);

    close_db();
    //delete_db_file(TEST_FILE_LOCATION);
}

void selects_performance_tests(uint32_t n) {
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

    for (uint32_t i = 0; i < 20; i++) {
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

    for (uint32_t i = 0; i < n; i++) {
        struct OperationPredicateParameter parameter = {
            .next = NULL,
            .column_name = "id",
            .predicate_operator = PO_EQUAL,
            .value_size = sizeof(int64_t),
            .value = &(int64_t) {i % 20},
        };

        struct SelectResultIterator iterator = operation_select(table_name, &parameter);

        while (iterator.has_more) {
            struct TableField *table_field = get_by_iterator(&iterator);
            free_table_row(table_field);
            iterator = get_next(&iterator);
        }
    }

    printf("Selection of %d rows was successful.\n", n);

    close_db();
    delete_db_file(TEST_FILE_LOCATION);
}

int main(int argc, char **argv) {
    if (argc != 3) {
        printf("Usage: %s <test_name> <test-arg>\n", argv[0]);
        return 1;
    }
    if (strcmp(argv[1], "insertions") == 0) {
        insertion_performance_tests(atoi(argv[2]));
        return 0;
    } else if (strcmp(argv[1], "updates") == 0) {
        update_performance_tests(atoi(argv[2]));
        return 0;
    } else if (strcmp(argv[1], "selects") == 0) {
        selects_performance_tests(atoi(argv[2]));
        return 0;
    } else {
        printf("Unknown test name: %s\n", argv[1]);
        return 1;
    }
}