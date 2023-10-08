//
// Created by ruskaof on 7/10/23.
//

#include "../include/table.h"
#include "../include/table_operations.h"
#include "../include/data_operations.h"
#include "../src/db/file.h"
#include "../src/db/table_metadata.h"
#include "../src/db/element.h"

#include <string.h>
#include <assert.h>
#include <stdio.h>

#if defined(__linux__) || defined(__APPLE__)

#define TEST_FILE_LOCATION "/home/ruskaof/Desktop/testdb2"

#endif

#if defined(_WIN32)

#define TEST_FILE_LOCATION "C:\\Users\\199-4\\Desktop\\testdb2"

#endif

void print_separator() {
    printf("-----------------------------------------------------------------------\n");
    printf("-----------------------------------------------------------------------\n");
}

void print_file() {
    print_separator();
    void *file_data_pointer;
    uint64_t file_size = get_file_size();
    mmap_file(&file_data_pointer, 0, file_size);

    struct FileHeader *file_header = (struct FileHeader *) file_data_pointer;
    printf("File header:\n");
    printf("Has deleted elements: %d\n", file_header->has_deleted_elements);
    printf("Last deleted element offset: %lu\n", file_header->last_deleted_element_offset);
    printf("Has table metadata elements: %d\n", file_header->has_table_metadata_elements);
    printf("Last table metadata element offset: %lu\n", file_header->last_table_metadata_element_offset);
    printf("Has table data elements: %d\n", file_header->has_table_data_elements);
    printf("Last table data element offset: %lu\n", file_header->last_table_data_element_offset);
    printf("Last element offset: %lu\n", file_header->last_element_offset);

    printf("\n");

    struct ElementHeader *element_header = (struct ElementHeader *) (file_data_pointer + FIRST_ELEMENT_OFFSET);
    uint64_t element_header_number = 0;

    while ((char *) element_header <= (char *) file_data_pointer + file_header->last_element_offset) {
        printf("Element header:\n");
        printf("Element number: %lu\n", element_header_number++);
        printf("Element offset: %lu\n", ((char *) element_header) - ((char *) file_data_pointer));
        printf("Element size: %lu\n", element_header->element_size);
        printf("Element type: %d\n", element_header->element_type);
        printf("Has next element of type: %d\n", element_header->has_next_element_of_type);
        printf("Next element of type offset: %lu\n", element_header->next_element_of_type_offset);
        printf("Has prev element of type: %d\n", element_header->has_prev_element_of_type);
        printf("Prev element of type offset: %lu\n", element_header->prev_element_of_type_offset);
        printf("Prev element offset: %lu\n", element_header->prev_element_offset);
        printf("\n");
        element_header = (struct ElementHeader *) (((char *) element_header) + element_header->element_size);
    }

    print_separator();
    munmap_file(file_data_pointer, file_size);
}

void table_operations_simple_insertions() {
    open_file(TEST_FILE_LOCATION);

    uint64_t first_table_columns_count = 2;
    struct TableColumn *columns = malloc(first_table_columns_count * sizeof(struct TableColumn));
    columns[0].type = TD_INT64;
    strcpy(columns[0].name, "test_column1");
    columns[1].type = TD_BOOL;
    strcpy(columns[1].name, "test_column2");
    operation_create_table("test_table1", columns, first_table_columns_count);
    free(columns);

    uint64_t second_table_columns_count = 3;
    columns = malloc(second_table_columns_count * sizeof(struct TableColumn));
    columns[0].type = TD_INT64;
    strcpy(columns[0].name, "test_column1");
    columns[1].type = TD_BOOL;
    strcpy(columns[1].name, "test_column2");
    columns[2].type = TD_STRING;
    strcpy(columns[2].name, "test_column3");
    operation_create_table("test_table2", columns, second_table_columns_count);
    free(columns);

    uint64_t table_metadata_element_offset;
    find_table_metadata_offset("test_table1", &table_metadata_element_offset);

    void *file_data_pointer;
    mmap_file(&file_data_pointer, 0, get_file_size());
    struct TableMetadataElement *table_metadata_element = (struct TableMetadataElement *) ((char *) file_data_pointer +
                                                                                           table_metadata_element_offset +
                                                                                           ELEMENT_VALUE_OFFSET);
    assert(strcmp(table_metadata_element->name, "test_table1") == 0);
    assert(table_metadata_element->columns_count == first_table_columns_count);
    assert(table_metadata_element->columns[0].type == TD_INT64);
    assert(strcmp(table_metadata_element->columns[0].name, "test_column1") == 0);
    assert(table_metadata_element->columns[1].type == TD_BOOL);
    assert(strcmp(table_metadata_element->columns[1].name, "test_column2") == 0);

    find_table_metadata_offset("test_table2", &table_metadata_element_offset);
    table_metadata_element = (struct TableMetadataElement *) ((char *) file_data_pointer +
                                                              table_metadata_element_offset + ELEMENT_VALUE_OFFSET);
    assert(strcmp(table_metadata_element->name, "test_table2") == 0);
    assert(table_metadata_element->columns_count == second_table_columns_count);
    assert(table_metadata_element->columns[0].type == TD_INT64);
    assert(strcmp(table_metadata_element->columns[0].name, "test_column1") == 0);
    assert(table_metadata_element->columns[1].type == TD_BOOL);
    assert(strcmp(table_metadata_element->columns[1].name, "test_column2") == 0);
    assert(table_metadata_element->columns[2].type == TD_STRING);
    assert(strcmp(table_metadata_element->columns[2].name, "test_column3") == 0);

    munmap_file(file_data_pointer, get_file_size());
    close_file();
    delete_file(TEST_FILE_LOCATION);
}

void table_operations_simple_deletions() {
    open_file(TEST_FILE_LOCATION);

    uint64_t first_table_columns_count = 2;
    struct TableColumn *columns = malloc(first_table_columns_count * sizeof(struct TableColumn));
    columns[0].type = TD_INT64;
    strcpy(columns[0].name, "test_column1");
    columns[1].type = TD_BOOL;
    strcpy(columns[1].name, "test_column2");
    operation_create_table("test_table1", columns, first_table_columns_count);
    free(columns);

    uint64_t second_table_columns_count = 3;
    columns = malloc(second_table_columns_count * sizeof(struct TableColumn));
    columns[0].type = TD_INT64;
    strcpy(columns[0].name, "test_column1");
    columns[1].type = TD_BOOL;
    strcpy(columns[1].name, "test_column2");
    columns[2].type = TD_STRING;
    strcpy(columns[2].name, "test_column3");
    operation_create_table("test_table2", columns, second_table_columns_count);
    free(columns);

    operation_drop_table("test_table1");

    uint64_t table_metadata_element_offset;
    assert(find_table_metadata_offset("test_table1", &table_metadata_element_offset) == -1);

    assert(find_table_metadata_offset("test_table2", &table_metadata_element_offset) == 0);

    operation_drop_table("test_table2");

    assert(find_table_metadata_offset("test_table2", &table_metadata_element_offset) == -1);


    void *file_data_pointer;
    mmap_file(&file_data_pointer, 0, get_file_size());
    struct FileHeader *file_header = (struct FileHeader *) file_data_pointer;
    assert(!file_header->has_table_metadata_elements);

    munmap_file(file_data_pointer, get_file_size());
    close_file();
    delete_file(TEST_FILE_LOCATION);
}

void table_operations_simple_insertions2() {
    open_file(TEST_FILE_LOCATION);

    uint64_t first_table_columns_count = 2;
    struct TableColumn *columns = malloc(first_table_columns_count * sizeof(struct TableColumn));
    columns[0].type = TD_INT64;
    strcpy(columns[0].name, "test_column1");
    columns[1].type = TD_BOOL;
    strcpy(columns[1].name, "test_column2");
    operation_create_table("test_table1", columns, first_table_columns_count);
    free(columns);

    uint64_t second_table_columns_count = 3;
    columns = malloc(second_table_columns_count * sizeof(struct TableColumn));
    columns[0].type = TD_INT64;
    strcpy(columns[0].name, "test_column1");
    columns[1].type = TD_BOOL;
    strcpy(columns[1].name, "test_column2");
    columns[2].type = TD_STRING;
    strcpy(columns[2].name, "test_column3");
    operation_create_table("test_table2", columns, second_table_columns_count);
    free(columns);

    uint64_t table_metadata_element_offset;
    find_table_metadata_offset("test_table1", &table_metadata_element_offset);

    void *file_data_pointer;
    mmap_file(&file_data_pointer, 0, get_file_size());
    struct TableMetadataElement *table_metadata_element = (struct TableMetadataElement *) ((char *) file_data_pointer +
                                                                                           table_metadata_element_offset +
                                                                                           ELEMENT_VALUE_OFFSET);
    assert(strcmp(table_metadata_element->name, "test_table1") == 0);
    assert(table_metadata_element->columns_count == first_table_columns_count);
    assert(table_metadata_element->columns[0].type == TD_INT64);
    assert(strcmp(table_metadata_element->columns[0].name, "test_column1") == 0);
    assert(table_metadata_element->columns[1].type == TD_BOOL);
    assert(strcmp(table_metadata_element->columns[1].name, "test_column2") == 0);

    find_table_metadata_offset("test_table2", &table_metadata_element_offset);
    table_metadata_element = (struct TableMetadataElement *) ((char *) file_data_pointer +
                                                              table_metadata_element_offset + ELEMENT_VALUE_OFFSET);
    assert(strcmp(table_metadata_element->name, "test_table2") == 0);
    assert(table_metadata_element->columns_count == second_table_columns_count);
    assert(table_metadata_element->columns[0].type == TD_INT64);
    assert(strcmp(table_metadata_element->columns[0].name, "test_column1") == 0);
    assert(table_metadata_element->columns[1].type == TD_BOOL);
    assert(strcmp(table_metadata_element->columns[1].name, "test_column2") == 0);
    assert(table_metadata_element->columns[2].type == TD_STRING);
    assert(strcmp(table_metadata_element->columns[2].name, "test_column3") == 0);

    munmap_file(file_data_pointer, get_file_size());
    close_file();
    delete_file(TEST_FILE_LOCATION);
}

void data_operations_simple_insertions() {
    open_file(TEST_FILE_LOCATION);

    uint64_t first_table_columns_count = 2;
    struct TableColumn *columns = malloc(first_table_columns_count * sizeof(struct TableColumn));
    columns[0].type = TD_INT64;
    strcpy(columns[0].name, "test_column1");
    columns[1].type = TD_BOOL;
    strcpy(columns[1].name, "test_column2");
    operation_create_table("test_table1", columns, first_table_columns_count);
    free(columns);

    uint64_t second_table_columns_count = 3;
    columns = malloc(second_table_columns_count * sizeof(struct TableColumn));
    columns[0].type = TD_INT64;
    strcpy(columns[0].name, "test_column1");
    columns[1].type = TD_BOOL;
    strcpy(columns[1].name, "test_column2");
    columns[2].type = TD_STRING;
    strcpy(columns[2].name, "test_column3");
    operation_create_table("test_table2", columns, second_table_columns_count);
    free(columns);

    // BOOL
    struct TableField *table1_row1_field2 = malloc(sizeof(struct TableField));
    table1_row1_field2->size = sizeof(bool);
    table1_row1_field2->next = NULL;
    bool *table1_row1_field2_value = malloc(sizeof(bool));
    *table1_row1_field2_value = true;
    table1_row1_field2->value = table1_row1_field2_value;

    // INT64
    struct TableField *table1_row1_field1 = malloc(sizeof(struct TableField));
    table1_row1_field1->size = sizeof(int64_t);
    table1_row1_field1->next = table1_row1_field2;
    int64_t *table1_row1_field1_value = malloc(sizeof(int64_t));
    *table1_row1_field1_value = 123;
    table1_row1_field1->value = table1_row1_field1_value;

    operation_insert("test_table1", table1_row1_field1);
    free(table1_row1_field1->value);
    free(table1_row1_field1);
    free(table1_row1_field2->value);
    free(table1_row1_field2);


    struct SelectResultIterator select_result_iterator = operation_select("test_table1", NULL);
    assert(select_result_iterator.has_element);
    assert(!select_result_iterator.has_more);

    struct TableField *table1_row1 = get_by_iterator(&select_result_iterator);
    assert(table1_row1->size == sizeof(int64_t));
    assert(table1_row1->next->size == sizeof(bool));
    assert(table1_row1->next->next == NULL);
    int64_t *table1_row1_field1_value2 = (int64_t *) table1_row1->value;
    assert(*table1_row1_field1_value2 == 123);
    bool *table1_row1_field2_value2 = (bool *) table1_row1->next->value;
    assert(*table1_row1_field2_value2 == true);

    free(table1_row1->value);
    free(table1_row1);

    // insert one more row to the first table
    struct TableField *table1_row2_field2 = malloc(sizeof(struct TableField));
    table1_row2_field2->size = sizeof(bool);
    table1_row2_field2->next = NULL;
    bool *table1_row2_field2_value = malloc(sizeof(bool));
    *table1_row2_field2_value = false;
    table1_row2_field2->value = table1_row2_field2_value;

    struct TableField *table1_row2_field1 = malloc(sizeof(struct TableField));
    table1_row2_field1->size = sizeof(int64_t);
    table1_row2_field1->next = table1_row2_field2;
    int64_t *table1_row2_field1_value = malloc(sizeof(int64_t));
    *table1_row2_field1_value = 456;
    table1_row2_field1->value = table1_row2_field1_value;

    operation_insert("test_table1", table1_row2_field1);
    free(table1_row2_field1->value);
    free(table1_row2_field1);
    free(table1_row2_field2->value);
    free(table1_row2_field2);

    select_result_iterator = operation_select("test_table1", NULL);
    assert(select_result_iterator.has_element);
    assert(select_result_iterator.has_more);

    struct TableField *table1_row2 = get_by_iterator(&select_result_iterator);
    assert(table1_row2->size == sizeof(int64_t));
    assert(table1_row2->next->size == sizeof(bool));
    assert(table1_row2->next->next == NULL);
    int64_t *table1_row2_field1_value2 = (int64_t *) table1_row2->value;
    assert(*table1_row2_field1_value2 == 456);
    bool *table1_row2_field2_value2 = (bool *) table1_row2->next->value;
    assert(*table1_row2_field2_value2 == false);

    free(table1_row2->value);
    free(table1_row2);

    select_result_iterator = get_next(&select_result_iterator);
    assert(select_result_iterator.has_element);
    assert(!select_result_iterator.has_more);

    struct TableField *table1_row1_2 = get_by_iterator(&select_result_iterator);
    assert(table1_row1_2->size == sizeof(int64_t));
    assert(table1_row1_2->next->size == sizeof(bool));
    assert(table1_row1_2->next->next == NULL);
    int64_t *table1_row1_field1_value3 = (int64_t *) table1_row1_2->value;
    assert(*table1_row1_field1_value3 == 123);
    bool *table1_row1_field2_value3 = (bool *) table1_row1_2->next->value;
    assert(*table1_row1_field2_value3 == true);

    free(table1_row1_2->value);
    free(table1_row1_2);

    close_file();
    delete_file(TEST_FILE_LOCATION);
}

void init_table_with_test_data() {
    uint64_t first_table_columns_count = 2;
    struct TableColumn *columns = malloc(first_table_columns_count * sizeof(struct TableColumn));
    columns[0].type = TD_INT64;
    strcpy(columns[0].name, "test_column1");
    columns[1].type = TD_BOOL;
    strcpy(columns[1].name, "test_column2");
    operation_create_table("test_table1", columns, first_table_columns_count);
    free(columns);

    uint64_t second_table_columns_count = 3;
    columns = malloc(second_table_columns_count * sizeof(struct TableColumn));
    columns[0].type = TD_INT64;
    strcpy(columns[0].name, "test_column1");
    columns[1].type = TD_BOOL;
    strcpy(columns[1].name, "test_column2");
    columns[2].type = TD_STRING;
    strcpy(columns[2].name, "test_column3");
    operation_create_table("test_table2", columns, second_table_columns_count);
    free(columns);

    // BOOL
    struct TableField *table1_row1_field2 = malloc(sizeof(struct TableField));
    table1_row1_field2->size = sizeof(bool);
    table1_row1_field2->next = NULL;
    bool *table1_row1_field2_value = malloc(sizeof(bool));
    *table1_row1_field2_value = true;
    table1_row1_field2->value = table1_row1_field2_value;

    // INT64
    struct TableField *table1_row1_field1 = malloc(sizeof(struct TableField));
    table1_row1_field1->size = sizeof(int64_t);
    table1_row1_field1->next = table1_row1_field2;
    int64_t *table1_row1_field1_value = malloc(sizeof(int64_t));
    *table1_row1_field1_value = 123;
    table1_row1_field1->value = table1_row1_field1_value;

    operation_insert("test_table1", table1_row1_field1);
    free(table1_row1_field1->value);
    free(table1_row1_field1);
    free(table1_row1_field2->value);
    free(table1_row1_field2);

    // insert one more row to the first table
    struct TableField *table1_row2_field2 = malloc(sizeof(struct TableField));
    table1_row2_field2->size = sizeof(bool);
    table1_row2_field2->next = NULL;
    bool *table1_row2_field2_value = malloc(sizeof(bool));
    *table1_row2_field2_value = false;
    table1_row2_field2->value = table1_row2_field2_value;

    struct TableField *table1_row2_field1 = malloc(sizeof(struct TableField));
    table1_row2_field1->size = sizeof(int64_t);
    table1_row2_field1->next = table1_row2_field2;
    int64_t *table1_row2_field1_value = malloc(sizeof(int64_t));
    *table1_row2_field1_value = 456;
    table1_row2_field1->value = table1_row2_field1_value;

    operation_insert("test_table1", table1_row2_field1);
    free(table1_row2_field1->value);
    free(table1_row2_field1);
    free(table1_row2_field2->value);
    free(table1_row2_field2);

    // insert 2 rows to the second table
// insert 3 kilobytes of string
    char *string_value = malloc(3 * 1024);
    memset(string_value, 'a', 3 * 1024);
    struct TableField *table2_row1_field3 = malloc(sizeof(struct TableField));
    table2_row1_field3->size = 3 * 1024;
    table2_row1_field3->next = NULL;
    table2_row1_field3->value = string_value;

    struct TableField *table2_row1_field2 = malloc(sizeof(struct TableField));
    table2_row1_field2->size = sizeof(bool);
    table2_row1_field2->next = table2_row1_field3;
    bool *table2_row1_field2_value = malloc(sizeof(bool));
    *table2_row1_field2_value = true;
    table2_row1_field2->value = table2_row1_field2_value;

    struct TableField *table2_row1_field1 = malloc(sizeof(struct TableField));
    table2_row1_field1->size = sizeof(int64_t);
    table2_row1_field1->next = table2_row1_field2;
    int64_t *table2_row1_field1_value = malloc(sizeof(int64_t));
    *table2_row1_field1_value = 123;
    table2_row1_field1->value = table2_row1_field1_value;

    operation_insert("test_table2", table2_row1_field1);
    free(table2_row1_field1->value);
    free(table2_row1_field1);
    free(table2_row1_field2->value);
    free(table2_row1_field2);
    free(table2_row1_field3->value);
    free(table2_row1_field3);

    // insert 2 kilobytes of string
    string_value = malloc(2 * 1024);
    memset(string_value, 'b', 2 * 1024);
    struct TableField *table2_row2_field3 = malloc(sizeof(struct TableField));
    table2_row2_field3->size = 2 * 1024;
    table2_row2_field3->next = NULL;
    table2_row2_field3->value = string_value;

    struct TableField *table2_row2_field2 = malloc(sizeof(struct TableField));
    table2_row2_field2->size = sizeof(bool);
    table2_row2_field2->next = table2_row2_field3;
    bool *table2_row2_field2_value = malloc(sizeof(bool));
    *table2_row2_field2_value = false;
    table2_row2_field2->value = table2_row2_field2_value;

    struct TableField *table2_row2_field1 = malloc(sizeof(struct TableField));
    table2_row2_field1->size = sizeof(int64_t);
    table2_row2_field1->next = table2_row2_field2;
    int64_t *table2_row2_field1_value = malloc(sizeof(int64_t));
    *table2_row2_field1_value = 456;
    table2_row2_field1->value = table2_row2_field1_value;

    operation_insert("test_table2", table2_row2_field1);
    free(table2_row2_field1->value);
    free(table2_row2_field1);
    free(table2_row2_field2->value);
    free(table2_row2_field2);
    free(table2_row2_field3->value);
    free(table2_row2_field3);
}

void data_operations_simple_insertions2() {
    open_file(TEST_FILE_LOCATION);

    init_table_with_test_data();

    // run assertions for second table
    struct SelectResultIterator select_result_iterator = operation_select("test_table2", NULL);
    assert(select_result_iterator.has_element);
    assert(select_result_iterator.has_more);

    struct TableField *table2_row2 = get_by_iterator(&select_result_iterator);
    assert(table2_row2->size == sizeof(int64_t));
    assert(table2_row2->next->size == sizeof(bool));
    assert(table2_row2->next->next->size == 2 * 1024);
    assert(table2_row2->next->next->next == NULL);
    int64_t *table2_row2_field1_value2 = (int64_t *) table2_row2->value;
    assert(*table2_row2_field1_value2 == 456);
    bool *table2_row2_field2_value2 = (bool *) table2_row2->next->value;
    assert(*table2_row2_field2_value2 == false);
    char *expected_string_value = malloc(2 * 1024);
    memset(expected_string_value, 'b', 2 * 1024);
    assert(memcmp(table2_row2->next->next->value, expected_string_value, 2 * 1024) == 0);
    free(expected_string_value);

    free(table2_row2->value);
    free(table2_row2);

    select_result_iterator = get_next(&select_result_iterator);
    assert(select_result_iterator.has_element);
    assert(!select_result_iterator.has_more);

    struct TableField *table2_row1 = get_by_iterator(&select_result_iterator);
    assert(table2_row1->size == sizeof(int64_t));
    assert(table2_row1->next->size == sizeof(bool));
    assert(table2_row1->next->next->size == 3 * 1024);
    assert(table2_row1->next->next->next == NULL);
    int64_t *table2_row1_field1_value2 = (int64_t *) table2_row1->value;
    assert(*table2_row1_field1_value2 == 123);
    bool *table2_row1_field2_value2 = (bool *) table2_row1->next->value;
    assert(*table2_row1_field2_value2 == true);
    expected_string_value = malloc(3 * 1024);
    memset(expected_string_value, 'a', 3 * 1024);
    assert(memcmp(table2_row1->next->next->value, expected_string_value, 3 * 1024) == 0);
    free(expected_string_value);

    free(table2_row1);

    // run select on second table with an int64 filter
    struct OperationPredicateParameter *predicate_parameter = malloc(sizeof(struct OperationPredicateParameter));
    strcpy(predicate_parameter->column_name, "test_column1");
    predicate_parameter->next = NULL;
    predicate_parameter->predicate_operator = PO_LESS_THAN;
    predicate_parameter->value = malloc(sizeof(int64_t));
    predicate_parameter->value_size = sizeof(int64_t);
    *((int64_t *) predicate_parameter->value) = 124;

    struct SelectResultIterator select_result_iterator2 = operation_select("test_table2", predicate_parameter);
    free(predicate_parameter->value);
    free(predicate_parameter);
    assert(select_result_iterator2.has_element);
    assert(!select_result_iterator2.has_more);

    struct TableField *table2_row1_2 = get_by_iterator(&select_result_iterator2);
    assert(table2_row1_2->size == sizeof(int64_t));
    assert(table2_row1_2->next->size == sizeof(bool));
    assert(table2_row1_2->next->next->size == 3 * 1024);
    assert(table2_row1_2->next->next->next == NULL);
    int64_t *table2_row1_field1_value3 = (int64_t *) table2_row1_2->value;
    assert(*table2_row1_field1_value3 == 123);
    bool *table2_row1_field2_value3 = (bool *) table2_row1_2->next->value;
    assert(*table2_row1_field2_value3 == true);
    expected_string_value = malloc(3 * 1024);
    memset(expected_string_value, 'a', 3 * 1024);
    assert(memcmp(table2_row1_2->next->next->value, expected_string_value, 3 * 1024) == 0);
    free(expected_string_value);
    free(table2_row1_2->value);
    free(table2_row1_2);

    close_file();
    delete_file(TEST_FILE_LOCATION);
}

void data_operations_with_deletions() {
    open_file(TEST_FILE_LOCATION);

    init_table_with_test_data();

    // delete first row from the second table where int value is 123
    struct OperationPredicateParameter *predicate_parameter = malloc(sizeof(struct OperationPredicateParameter));
    strcpy(predicate_parameter->column_name, "test_column1");
    predicate_parameter->next = NULL;
    predicate_parameter->predicate_operator = PO_EQUAL;
    predicate_parameter->value = malloc(sizeof(int64_t));
    predicate_parameter->value_size = sizeof(int64_t);
    *((int64_t *) predicate_parameter->value) = 123;

    operation_delete("test_table2", predicate_parameter);

    free(predicate_parameter->value);
    free(predicate_parameter);

    // run select on second table and check that there is no row with int value 123

    struct SelectResultIterator select_result_iterator2 = operation_select("test_table2", NULL);
    assert(select_result_iterator2.has_element);
    assert(!select_result_iterator2.has_more);

    struct TableField *table2_row2 = get_by_iterator(&select_result_iterator2);
    assert(table2_row2->size == sizeof(int64_t));
    assert(table2_row2->next->size == sizeof(bool));
    assert(table2_row2->next->next->size == 2 * 1024);
    assert(table2_row2->next->next->next == NULL);
    int64_t *table2_row2_field1_value2 = (int64_t *) table2_row2->value;
    assert(*table2_row2_field1_value2 == 456);
    bool *table2_row2_field2_value2 = (bool *) table2_row2->next->value;
    assert(*table2_row2_field2_value2 == false);
    char *expected_string_value = malloc(2 * 1024);
    memset(expected_string_value, 'b', 2 * 1024);
    assert(memcmp(table2_row2->next->next->value, expected_string_value, 2 * 1024) == 0);
    free(expected_string_value);

    free(table2_row2->value);
    free(table2_row2);
}

int main() {
    table_operations_simple_insertions();
    print_separator();
    table_operations_simple_deletions();
    print_separator();
    table_operations_simple_insertions2();
    print_separator();
    data_operations_simple_insertions();
    print_separator();
    data_operations_simple_insertions2();
    print_separator();
    data_operations_with_deletions();

    printf("\033[0;32m");
    printf("All tests for %s passed!\n", __FILE__);
    printf("\033[0m");
}