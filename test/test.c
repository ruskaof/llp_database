//
// Created by d.rusinov on 23.09.2023.
//

#include <stdio.h>
#include <string.h>

#include "../src/db/file.h"
#include "../src/db/allocator.h"
#include "../src/db/page.h"
#include "../include/table_operations.h"
#include "../src/db/table_metadata.h"

void allocator_test() {
    int fd = open_file("/home/ruskaof/Desktop/testdb2");
    uint64_t first_page_offset;
    uint64_t first_page_size;
    uint64_t first_alloc_result = allocate_page(fd, 123, PT_TABLE_DATA_PAGE, &first_page_offset, &first_page_size);
    printf("First alloc result: %ld, offset: %ld\n", first_alloc_result, first_page_offset);

    uint64_t second_page_offset;
    uint64_t second_page_size;
    uint64_t second_alloc_result = allocate_page(fd, 123, PT_TABLE_METADATA_PAGE, &second_page_offset,
                                                 &second_page_size);
    printf("Second alloc result: %ld, offset: %ld\n", second_alloc_result, second_page_offset);

    uint64_t third_page_offset;
    uint64_t third_page_size;
    uint64_t third_alloc_result = allocate_page(fd, 123, PT_TABLE_METADATA_PAGE, &third_page_offset, &third_page_size);
    printf("Third alloc result: %ld, offset: %ld\n", third_alloc_result, third_page_offset);

    uint64_t fourth_page_offset;
    uint64_t fourth_page_size;
    uint64_t fourth_alloc_result = allocate_page(fd, 123, PT_TABLE_METADATA_PAGE, &fourth_page_offset,
                                                 &fourth_page_size);
    printf("Fourth alloc result: %ld, offset: %ld\n", fourth_alloc_result, fourth_page_offset);

    // delete second page
    int delete_result = delete_page(fd, second_page_offset);
    delete_result = delete_page(fd, third_page_offset);


    uint64_t fifth_page_offset;
    uint64_t fifth_page_size;
    uint64_t fifth_alloc_result = allocate_page(fd, 123, PT_TABLE_DATA_PAGE, &fifth_page_offset, &fifth_page_size);
    printf("Fifth alloc result: %ld, offset: %ld\n", fifth_alloc_result, fifth_page_offset);

    delete_file("/home/ruskaof/Desktop/testdb2");
}

//void table_metadata_test() {
//    int fd = open_file("/home/ruskaof/Desktop/testdb3");
//
//    struct TableColumn *first_columns = malloc(sizeof(struct TableColumn) * 2);
//    strcpy(first_columns[0].name, "id1");
//    first_columns[0].type = TD_INT64;
//
//    strcpy(first_columns[1].name, "name1");
//    first_columns[1].type = TD_INT64;
//
//    int create_result = operation_create_table(fd, "first_test_table", first_columns, 2);
//    printf("Create result: %d\n", create_result);
//
//    struct TableColumn *second_columns = malloc(sizeof(struct TableColumn) * 3);
//    strcpy(first_columns[0].name, "id2");
//    first_columns[0].type = TD_INT64;
//
//    strcpy(first_columns[1].name, "name2");
//    first_columns[1].type = TD_INT64;
//
//    strcpy(first_columns[1].name, "bf2");
//    first_columns[1].type = TD_BOOL;
//
//    int create_result_2 = operation_create_table(fd, "second_test_table", second_columns, 3);
//    printf("Create result: %d\n", create_result_2);
//
//    uint64_t table_metadata_offset1;
//    uint64_t table_metadata_size1;
//    int find_result1 = find_table_metadata(fd, "first_test_table", &table_metadata_offset1, &table_metadata_size1);
//    printf("Find result: %d, offset: %ld, size: %ld\n", find_result1, table_metadata_offset1, table_metadata_size1);
//
//    uint64_t table_metadata_offset2;
//    uint64_t table_metadata_size2;
//    int find_result2 = find_table_metadata(fd, "second_test_table", &table_metadata_offset2, &table_metadata_size2);
//    printf("Find result: %d, offset: %ld, size: %ld\n", find_result2, table_metadata_offset2, table_metadata_size2);
//
//    delete_file("/home/ruskaof/Desktop/testdb3");
//}

int main() {
    allocator_test();
    //table_metadata_test();
}