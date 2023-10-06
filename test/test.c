//
// Created by d.rusinov on 23.09.2023.
//

#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "../src/db/file.h"
#include "../src/db/allocator.h"
#include "../src/db/element.h"
#include "../include/table_operations.h"
#include "../src/db/table_metadata.h"

void print_file(int fd) {
    void *file_data_pointer;
    uint64_t file_size = get_file_size(fd);
    mmap_file(fd, &file_data_pointer, 0, file_size);

    struct FileHeader *file_header = (struct FileHeader *) file_data_pointer;
    printf("File header:\n");
    printf("Has deleted elements: %d\n", file_header->has_deleted_elements);
    printf("First deleted element offset: %lu\n", file_header->first_deleted_element_offset);
    printf("Has table metadata elements: %d\n", file_header->has_table_metadata_elements);
    printf("Last table metadata element offset: %lu\n", file_header->last_table_metadata_element_offset);
    printf("Has table data elements: %d\n", file_header->has_table_data_elements);
    printf("Last table data element offset: %lu\n", file_header->last_table_data_element_offset);
    printf("Last element offset: %lu\n", file_header->last_element_offset);

    struct ElementHeader *element_header = (struct ElementHeader *) (file_data_pointer + FIRST_ELEMENT_OFFSET);
    while ((char *) element_header <= (char *) file_data_pointer + file_header->last_element_offset) {
        printf("Element header:\n");
        printf("Element offset: %lu\n", ((char *) element_header) - ((char *) file_data_pointer));
        printf("Element size: %lu\n", element_header->element_size);
        printf("Element type: %d\n", element_header->element_type);
        printf("Has next element of type: %d\n", element_header->has_next_element_of_type);
        printf("Next element of type offset: %lu\n", element_header->next_element_of_type_offset);
        printf("Has prev element of type: %d\n", element_header->has_prev_element_of_type);
        printf("Prev element of type offset: %lu\n", element_header->prev_element_of_type_offset);
        printf("Has prev element: %d\n", element_header->has_prev_element);
        printf("Prev element offset: %lu\n", element_header->prev_element_offset);
        printf("\n");
        element_header = (struct ElementHeader *) (((char *) element_header) + element_header->element_size);
    }

    munmap_file(file_data_pointer, file_size, fd);
}

void allocator_test() {
    int fd = open_file("/home/ruskaof/Desktop/testdb2");
    void *file_data_pointer;

    uint64_t first_allocated_element_offset;
    allocate_element(fd, 100, ET_TABLE_DATA, &first_allocated_element_offset);

    uint64_t file_size = get_file_size(fd);
    mmap_file(fd, &file_data_pointer, 0, file_size);
    struct FileHeader *file_header = (struct FileHeader *) file_data_pointer;
    assert(file_header->has_table_data_elements);
    assert(file_header->last_table_data_element_offset == first_allocated_element_offset);
    struct ElementHeader *first_element_header = (struct ElementHeader *) (file_data_pointer + FIRST_ELEMENT_OFFSET);
    assert(first_element_header->element_size == MIN_ELEMENT_SIZE);
    assert(first_element_header->element_type == ET_TABLE_DATA);
    assert(first_element_header->has_next_element_of_type == false);
    assert(first_element_header->has_prev_element_of_type == false);
    assert(first_element_header->has_prev_element == false);

    munmap_file(file_data_pointer, file_size, fd);

    uint64_t second_allocated_element_offset;
    allocate_element(fd, 200, ET_TABLE_DATA, &second_allocated_element_offset);

    file_size = get_file_size(fd);
    mmap_file(fd, &file_data_pointer, 0, file_size);
    file_header = (struct FileHeader *) file_data_pointer;
    assert(file_header->has_table_data_elements);
    assert(file_header->last_table_data_element_offset == second_allocated_element_offset);
    first_element_header = (struct ElementHeader *) (file_data_pointer + FIRST_ELEMENT_OFFSET);
    assert(first_element_header->element_size == MIN_ELEMENT_SIZE);
    assert(first_element_header->element_type == ET_TABLE_DATA);
    assert(first_element_header->has_next_element_of_type == true);
    assert(first_element_header->next_element_of_type_offset == second_allocated_element_offset);
    assert(first_element_header->has_prev_element_of_type == false);
    assert(first_element_header->has_prev_element == false);
    struct ElementHeader *second_element_header = (struct ElementHeader *) (((char *) first_element_header) +
                                                                            first_element_header->element_size);
    assert(second_element_header->element_size == MIN_ELEMENT_SIZE);
    assert(second_element_header->element_type == ET_TABLE_DATA);
    assert(second_element_header->has_next_element_of_type == false);
    assert(second_element_header->has_prev_element_of_type == true);
    assert(second_element_header->prev_element_of_type_offset == first_allocated_element_offset);
    assert(second_element_header->has_prev_element == true);

    uint64_t third_allocated_element_offset;
    allocate_element(fd, 300, ET_TABLE_METADATA, &third_allocated_element_offset);

    file_size = get_file_size(fd);
    mmap_file(fd, &file_data_pointer, 0, file_size);
    file_header = (struct FileHeader *) file_data_pointer;
    assert(file_header->has_table_metadata_elements);
    assert(file_header->last_table_metadata_element_offset == third_allocated_element_offset);
    struct ElementHeader *third_element_header = (struct ElementHeader *) (((char *) second_element_header) +
                                                                           second_element_header->element_size);
    assert(third_element_header->element_size == MIN_ELEMENT_SIZE);
    assert(third_element_header->element_type == ET_TABLE_METADATA);
    assert(third_element_header->has_next_element_of_type == false);
    assert(third_element_header->has_prev_element_of_type == false);
    assert(third_element_header->has_prev_element == true);
    assert(third_element_header->prev_element_offset == second_allocated_element_offset);

    munmap_file(file_data_pointer, file_size, fd);

    uint64_t fourth_allocated_element_offset;
    allocate_element(fd, 400, ET_TABLE_METADATA, &fourth_allocated_element_offset);

    file_size = get_file_size(fd);
    mmap_file(fd, &file_data_pointer, 0, file_size);
    file_header = (struct FileHeader *) file_data_pointer;
    assert(file_header->has_table_metadata_elements);
    assert(file_header->last_table_metadata_element_offset == fourth_allocated_element_offset);
    third_element_header = (struct ElementHeader *) (((char *) second_element_header) +
                                                     second_element_header->element_size);
    assert(third_element_header->element_size == MIN_ELEMENT_SIZE);
    assert(third_element_header->element_type == ET_TABLE_METADATA);
    assert(third_element_header->has_next_element_of_type == true);
    assert(third_element_header->next_element_of_type_offset == fourth_allocated_element_offset);
    assert(third_element_header->has_prev_element_of_type == false);
    assert(third_element_header->has_prev_element == true);
    struct ElementHeader *fourth_element_header = (struct ElementHeader *) (((char *) third_element_header) +
                                                                            third_element_header->element_size);
    assert(fourth_element_header->element_size == MIN_ELEMENT_SIZE);
    assert(fourth_element_header->element_type == ET_TABLE_METADATA);
    assert(fourth_element_header->has_next_element_of_type == false);
    assert(fourth_element_header->has_prev_element_of_type == true);
    assert(fourth_element_header->prev_element_of_type_offset == third_allocated_element_offset);
    assert(fourth_element_header->has_prev_element == true);
    assert(fourth_element_header->prev_element_offset == third_allocated_element_offset);

    munmap_file(file_data_pointer, file_size, fd);

    delete_element(fd, second_allocated_element_offset);

    file_size = get_file_size(fd);
    mmap_file(fd, &file_data_pointer, 0, file_size);
    file_header = (struct FileHeader *) file_data_pointer;
    assert(file_header->has_table_data_elements);
    assert(file_header->last_table_data_element_offset == first_allocated_element_offset);
    assert(file_header->has_deleted_elements);
    assert(file_header->first_deleted_element_offset == second_allocated_element_offset);
    first_element_header = (struct ElementHeader *) (file_data_pointer + FIRST_ELEMENT_OFFSET);
    assert(first_element_header->element_size == MIN_ELEMENT_SIZE);
    assert(first_element_header->element_type == ET_TABLE_DATA);
    assert(first_element_header->has_next_element_of_type == false);
    assert(first_element_header->has_prev_element_of_type == false);
    assert(first_element_header->has_prev_element == false);
    second_element_header = (struct ElementHeader *) (((char *) first_element_header) +
                                                      first_element_header->element_size);
    assert(second_element_header->element_size == MIN_ELEMENT_SIZE);
    assert(second_element_header->element_type == ET_DELETED);
    assert(second_element_header->has_next_element_of_type == false);
    assert(second_element_header->has_prev_element_of_type == false);
    assert(second_element_header->has_prev_element == true);
    assert(second_element_header->prev_element_offset == first_allocated_element_offset);

    munmap_file(file_data_pointer, file_size, fd);

    delete_element(fd, third_allocated_element_offset);

    file_size = get_file_size(fd);
    mmap_file(fd, &file_data_pointer, 0, file_size);
    file_header = (struct FileHeader *) file_data_pointer;
    assert(file_header->has_table_metadata_elements);
    assert(file_header->last_table_metadata_element_offset == fourth_allocated_element_offset);
    assert(file_header->has_deleted_elements);
    assert(file_header->first_deleted_element_offset == second_allocated_element_offset);
    second_element_header = (struct ElementHeader *) (((char *) first_element_header) +
                                                      first_element_header->element_size);
    //assert(second_element_header->element_size == MIN_ELEMENT_SIZE * 2 - sizeof(struct ElementHeader));
    assert(second_element_header->element_type == ET_DELETED);
    assert(second_element_header->has_next_element_of_type == false);
    assert(second_element_header->has_prev_element_of_type == false);
    assert(second_element_header->has_prev_element == true);
    assert(second_element_header->prev_element_offset == first_allocated_element_offset);
    third_element_header = (struct ElementHeader *) (((char *) second_element_header) +
                                                     second_element_header->element_size);
    assert(third_element_header->element_size == MIN_ELEMENT_SIZE);
    assert(third_element_header->element_type == ET_TABLE_METADATA);
    assert(third_element_header->has_next_element_of_type == false);
    assert(third_element_header->has_prev_element_of_type == false);
    assert(third_element_header->has_prev_element == true);
    assert(third_element_header->prev_element_offset == second_allocated_element_offset);

    print_file(fd);

    close_file(fd);

    delete_file("/home/ruskaof/Desktop/testdb2");
}


int main() {
    allocator_test();
}