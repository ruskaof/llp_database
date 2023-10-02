//
// Created by d.rusinov on 23.09.2023.
//

#include <stdio.h>
#include <string.h>

#include "../src/db/file.h"
#include "../src/db/allocator.h"
#include "../src/db/page.h"

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

    // delete second page
    int delete_result = delete_page(fd, second_page_offset);

    uint64_t fourth_page_offset;
    uint64_t fourth_page_size;
    uint64_t fourth_alloc_result = allocate_page(fd, 123, PT_TABLE_DATA_PAGE, &fourth_page_offset, &fourth_page_size);
    printf("Fourth alloc result: %ld, offset: %ld\n", fourth_alloc_result, fourth_page_offset);

    delete_file("/home/ruskaof/Desktop/testdb2");
}

int main() {
    allocator_test();
    //schema_create_test();
}