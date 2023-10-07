//
// Created by ruskaof on 7/10/23.
//

#include "../include/table.h"
#include "../include/table_operations.h"
#include "../src/db/file.h"
#include "../src/db/table_metadata.h"
#include "../src/db/element.h"

#include <string.h>
#include <assert.h>
#include <stdio.h>

#define TEST_FILE_LOCATION "/home/ruskaof/Desktop/testdb2"

void print_separator() {
    printf("-----------------------------------------------------------------------\n");
    printf("-----------------------------------------------------------------------\n");
}

void print_file(int fd) {
    print_separator();
    void *file_data_pointer;
    uint64_t file_size = get_file_size(fd);
    mmap_file(fd, &file_data_pointer, 0, file_size);

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
    munmap_file(file_data_pointer, file_size, fd);
}

void table_operations_simple_insertions() {
    int fd = open_file(TEST_FILE_LOCATION);

    uint64_t first_table_columns_count = 2;
    struct TableColumn *columns = malloc(first_table_columns_count * sizeof(struct TableColumn));
    columns[0].type = TD_INT64;
    strcpy(columns[0].name, "test_column1");
    columns[1].type = TD_BOOL;
    strcpy(columns[1].name, "test_column2");
    operation_create_table(fd, "test_table1", columns, first_table_columns_count);
    free(columns);

    uint64_t second_table_columns_count = 3;
    columns = malloc(second_table_columns_count * sizeof(struct TableColumn));
    columns[0].type = TD_INT64;
    strcpy(columns[0].name, "test_column1");
    columns[1].type = TD_BOOL;
    strcpy(columns[1].name, "test_column2");
    columns[2].type = TD_STRING;
    strcpy(columns[2].name, "test_column3");
    operation_create_table(fd, "test_table2", columns, second_table_columns_count);
    free(columns);

    uint64_t table_metadata_element_offset;
    find_table_metadata_offset(fd, "test_table1", &table_metadata_element_offset);

    void *file_data_pointer;
    mmap_file(fd, &file_data_pointer, 0, get_file_size(fd));
    struct TableMetadataElement *table_metadata_element = (struct TableMetadataElement *) ((char *) file_data_pointer +
                                                                                           table_metadata_element_offset +
                                                                                           ELEMENT_SUBHEADER_OFFSET);
    assert(strcmp(table_metadata_element->name, "test_table1") == 0);
    assert(table_metadata_element->columns_count == first_table_columns_count);
    assert(table_metadata_element->columns[0].type == TD_INT64);
    assert(strcmp(table_metadata_element->columns[0].name, "test_column1") == 0);
    assert(table_metadata_element->columns[1].type == TD_BOOL);
    assert(strcmp(table_metadata_element->columns[1].name, "test_column2") == 0);

    find_table_metadata_offset(fd, "test_table2", &table_metadata_element_offset);
    table_metadata_element = (struct TableMetadataElement *) ((char *) file_data_pointer +
                                                              table_metadata_element_offset + ELEMENT_SUBHEADER_OFFSET);
    assert(strcmp(table_metadata_element->name, "test_table2") == 0);
    assert(table_metadata_element->columns_count == second_table_columns_count);
    assert(table_metadata_element->columns[0].type == TD_INT64);
    assert(strcmp(table_metadata_element->columns[0].name, "test_column1") == 0);
    assert(table_metadata_element->columns[1].type == TD_BOOL);
    assert(strcmp(table_metadata_element->columns[1].name, "test_column2") == 0);
    assert(table_metadata_element->columns[2].type == TD_STRING);
    assert(strcmp(table_metadata_element->columns[2].name, "test_column3") == 0);

    munmap_file(file_data_pointer, get_file_size(fd), fd);
    close_file(fd);
    delete_file(TEST_FILE_LOCATION);
}

void table_operations_simple_deletions() {
    int fd = open_file(TEST_FILE_LOCATION);

    uint64_t first_table_columns_count = 2;
    struct TableColumn *columns = malloc(first_table_columns_count * sizeof(struct TableColumn));
    columns[0].type = TD_INT64;
    strcpy(columns[0].name, "test_column1");
    columns[1].type = TD_BOOL;
    strcpy(columns[1].name, "test_column2");
    operation_create_table(fd, "test_table1", columns, first_table_columns_count);
    free(columns);

    uint64_t second_table_columns_count = 3;
    columns = malloc(second_table_columns_count * sizeof(struct TableColumn));
    columns[0].type = TD_INT64;
    strcpy(columns[0].name, "test_column1");
    columns[1].type = TD_BOOL;
    strcpy(columns[1].name, "test_column2");
    columns[2].type = TD_STRING;
    strcpy(columns[2].name, "test_column3");
    operation_create_table(fd, "test_table2", columns, second_table_columns_count);
    free(columns);

    operation_drop_table(fd, "test_table1");

    uint64_t table_metadata_element_offset;
    assert(find_table_metadata_offset(fd, "test_table1", &table_metadata_element_offset) == -1);

    assert(find_table_metadata_offset(fd, "test_table2", &table_metadata_element_offset) == 0);

    operation_drop_table(fd, "test_table2");

    assert(find_table_metadata_offset(fd, "test_table2", &table_metadata_element_offset) == -1);


    void *file_data_pointer;
    mmap_file(fd, &file_data_pointer, 0, get_file_size(fd));
    struct FileHeader *file_header = (struct FileHeader *) file_data_pointer;
    assert(!file_header->has_table_metadata_elements);

    munmap_file(file_data_pointer, get_file_size(fd), fd);
    close_file(fd);
    delete_file(TEST_FILE_LOCATION);
}

int main() {
    table_operations_simple_insertions();
    table_operations_simple_deletions();
}