//
// Created by d.rusinov on 23.09.2023.
//

#include <stdio.h>
#include <assert.h>

#include "../src/db/file_private.h"
#include "../src/db/element_allocator.h"

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

//void print_file(int fd) {
//    print_separator();
//    void *file_data_pointer;
//    uint64_t file_size = get_file_size(fd);
//    mmap_file(fd, &file_data_pointer, 0, file_size);
//
//    struct FileHeader *file_header = (struct FileHeader *) file_data_pointer;
//    printf("File header:\n");
//    printf("Has deleted elements: %d\n", file_header->has_deleted_elements);
//    printf("Last deleted element offset: %lu\n", file_header->last_deleted_element_offset);
//    printf("Has table metadata elements: %d\n", file_header->has_table_metadata_elements);
//    printf("Last table metadata element offset: %lu\n", file_header->last_table_metadata_element_offset);
//    printf("Has table data elements: %d\n", file_header->has_table_data_elements);
//    printf("Last table data element offset: %lu\n", file_header->last_table_data_element_offset);
//    printf("Last element offset: %lu\n", file_header->last_element_offset);
//
//    printf("\n");
//
//    struct ElementHeader *element_header = (struct ElementHeader *) (file_data_pointer + FIRST_ELEMENT_OFFSET);
//    uint64_t element_header_number = 0;
//
//    while ((char *) element_header <= (char *) file_data_pointer + file_header->last_element_offset) {
//        printf("Element header:\n");
//        printf("Element number: %lu\n", element_header_number++);
//        printf("Element offset: %lu\n", ((char *) element_header) - ((char *) file_data_pointer));
//        printf("Element size: %lu\n", element_header->element_size);
//        printf("Element type: %d\n", element_header->element_type);
//        printf("Has next element of type: %d\n", element_header->has_next_element_of_type);
//        printf("Next element of type offset: %lu\n", element_header->next_element_of_type_offset);
//        printf("Has prev element of type: %d\n", element_header->has_prev_element_of_type);
//        printf("Prev element of type offset: %lu\n", element_header->prev_element_of_type_offset);
//        printf("Prev element offset: %lu\n", element_header->prev_element_offset);
//        printf("\n");
//        element_header = (struct ElementHeader *) (((char *) element_header) + element_header->element_size);
//    }
//
//    print_separator();
//    munmap_file(file_data_pointer, file_size, fd);
//}

void assert_element_on_offset(void *file_data_pointer,
                              uint64_t element_offset,
                              uint64_t element_size,
                              enum ElementType element_type,
                              bool has_next_element_of_type,
                              uint64_t next_element_of_type_offset,
                              bool has_prev_element_of_type,
                              uint64_t prev_element_of_type_offset,
                              uint64_t prev_element_offset) {
    struct ElementHeader *element_header = (struct ElementHeader *) (((char *) file_data_pointer) + element_offset);
    assert(element_header->element_size == element_size);
    assert(element_header->element_type == element_type);
    assert(element_header->has_next_element_of_type == has_next_element_of_type);
    if (has_next_element_of_type) {
        assert(element_header->next_element_of_type_offset == next_element_of_type_offset);
    }
    assert(element_header->has_prev_element_of_type == has_prev_element_of_type);
    if (has_prev_element_of_type) {
        assert(element_header->prev_element_of_type_offset == prev_element_of_type_offset);
    }

    if (element_offset != FIRST_ELEMENT_OFFSET) {
        assert(element_header->prev_element_offset == prev_element_offset);
    }
}

void allocator_test_single_type() {
    open_file(TEST_FILE_LOCATION);
    uint64_t first_allocated_element_offset;
    allocate_element(100, ET_TABLE_DATA, &first_allocated_element_offset);

    uint64_t file_size = get_file_size();
    void *file_data_pointer;
    mmap_file(&file_data_pointer, 0, file_size);
    struct FileHeader *file_header = (struct FileHeader *) file_data_pointer;
    assert(file_header->last_element_offset == first_allocated_element_offset);
    assert(file_header->has_table_data_elements);
    assert(file_header->last_table_data_element_offset == first_allocated_element_offset);
    assert_element_on_offset(file_data_pointer,
                             FIRST_ELEMENT_OFFSET,
                             MIN_ELEMENT_SIZE,
                             ET_TABLE_DATA, false, 0,
                             false, 0, 0);
    munmap_file(file_data_pointer, file_size);

    uint64_t second_allocated_element_offset;
    allocate_element(200, ET_TABLE_DATA, &second_allocated_element_offset);

    file_size = get_file_size();
    mmap_file(&file_data_pointer, 0, file_size);
    file_header = (struct FileHeader *) file_data_pointer;
    assert(file_header->last_element_offset == second_allocated_element_offset);
    assert(file_header->has_table_data_elements);
    assert(file_header->last_table_data_element_offset == second_allocated_element_offset);
    assert_element_on_offset(file_data_pointer,
                             first_allocated_element_offset,
                             MIN_ELEMENT_SIZE,
                             ET_TABLE_DATA,
                             true, second_allocated_element_offset,
                             false, 0, 0);
    assert_element_on_offset(file_data_pointer,
                             second_allocated_element_offset,
                             MIN_ELEMENT_SIZE,
                             ET_TABLE_DATA,
                             false, 0,
                             true, first_allocated_element_offset, first_allocated_element_offset);
    munmap_file(file_data_pointer, file_size);

    uint64_t third_allocated_element_offset;
    allocate_element(300, ET_TABLE_DATA, &third_allocated_element_offset);

    file_size = get_file_size();
    mmap_file(&file_data_pointer, 0, file_size);
    file_header = (struct FileHeader *) file_data_pointer;
    assert(file_header->last_element_offset == third_allocated_element_offset);
    assert(file_header->has_table_data_elements);
    assert(file_header->last_table_data_element_offset == third_allocated_element_offset);
    assert_element_on_offset(file_data_pointer,
                             first_allocated_element_offset,
                             MIN_ELEMENT_SIZE,
                             ET_TABLE_DATA,
                             true, second_allocated_element_offset,
                             false, 0, 0);
    assert_element_on_offset(file_data_pointer,
                             second_allocated_element_offset,
                             MIN_ELEMENT_SIZE,
                             ET_TABLE_DATA,
                             true, third_allocated_element_offset,
                             true, first_allocated_element_offset, first_allocated_element_offset);
    assert_element_on_offset(file_data_pointer,
                             third_allocated_element_offset,
                             MIN_ELEMENT_SIZE,
                             ET_TABLE_DATA,
                             false, 0,
                             true, second_allocated_element_offset, second_allocated_element_offset);
    munmap_file(file_data_pointer, file_size);

    delete_element(second_allocated_element_offset);

    file_size = get_file_size();
    mmap_file(&file_data_pointer, 0, file_size);
    file_header = (struct FileHeader *) file_data_pointer;
    assert(file_header->last_element_offset == third_allocated_element_offset);
    assert(file_header->has_table_data_elements);
    assert(file_header->last_table_data_element_offset == third_allocated_element_offset);
    assert_element_on_offset(file_data_pointer,
                             first_allocated_element_offset,
                             MIN_ELEMENT_SIZE,
                             ET_TABLE_DATA,
                             true, third_allocated_element_offset,
                             false, 0, 0);
    assert_element_on_offset(file_data_pointer,
                             second_allocated_element_offset,
                             MIN_ELEMENT_SIZE,
                             ET_DELETED,
                             false, 0,
                             false, 0, first_allocated_element_offset);
    assert_element_on_offset(file_data_pointer,
                             third_allocated_element_offset,
                             MIN_ELEMENT_SIZE,
                             ET_TABLE_DATA,
                             false, 0,
                             true, first_allocated_element_offset, second_allocated_element_offset);

    munmap_file(file_data_pointer, file_size);
    close_file();
    delete_file(TEST_FILE_LOCATION);
}

void allocator_test_multiple_types() {
    open_file(TEST_FILE_LOCATION);
    uint64_t first_allocated_element_offset;
    allocate_element(100, ET_TABLE_DATA, &first_allocated_element_offset);
    // table_data

    uint64_t file_size = get_file_size();
    void *file_data_pointer;
    mmap_file(&file_data_pointer, 0, file_size);
    struct FileHeader *file_header = (struct FileHeader *) file_data_pointer;
    assert(file_header->last_element_offset == first_allocated_element_offset);
    assert(file_header->has_table_data_elements);
    assert(file_header->last_table_data_element_offset == first_allocated_element_offset);
    assert_element_on_offset(file_data_pointer,
                             FIRST_ELEMENT_OFFSET,
                             MIN_ELEMENT_SIZE,
                             ET_TABLE_DATA,
                             false, 0,
                             false, 0, 0);
    munmap_file(file_data_pointer, file_size);

    uint64_t second_allocated_element_offset;
    allocate_element(200, ET_TABLE_METADATA, &second_allocated_element_offset);
    // table_data -> table_metadata

    file_size = get_file_size();
    mmap_file(&file_data_pointer, 0, file_size);
    file_header = (struct FileHeader *) file_data_pointer;
    assert(file_header->last_element_offset == second_allocated_element_offset);
    assert(file_header->has_table_data_elements);
    assert(file_header->last_table_data_element_offset == first_allocated_element_offset);
    assert(file_header->has_table_metadata_elements);
    assert(file_header->last_table_metadata_element_offset == second_allocated_element_offset);
    assert_element_on_offset(file_data_pointer,
                             FIRST_ELEMENT_OFFSET,
                             MIN_ELEMENT_SIZE,
                             ET_TABLE_DATA,
                             false, 0,
                             false, 0, 0);
    assert_element_on_offset(file_data_pointer,
                             second_allocated_element_offset,
                             MIN_ELEMENT_SIZE,
                             ET_TABLE_METADATA,
                             false, 0,
                             false, 0, first_allocated_element_offset);
    munmap_file(file_data_pointer, file_size);

    uint64_t third_allocated_element_offset;
    allocate_element(300, ET_TABLE_DATA, &third_allocated_element_offset);
    // table_data -> table_metadata -> table_data

    file_size = get_file_size();
    mmap_file(&file_data_pointer, 0, file_size);
    file_header = (struct FileHeader *) file_data_pointer;
    assert(file_header->last_element_offset == third_allocated_element_offset);
    assert(file_header->has_table_data_elements);
    assert(file_header->last_table_data_element_offset == third_allocated_element_offset);
    assert(file_header->has_table_metadata_elements);
    assert(file_header->last_table_metadata_element_offset == second_allocated_element_offset);
    assert_element_on_offset(file_data_pointer,
                             FIRST_ELEMENT_OFFSET,
                             MIN_ELEMENT_SIZE,
                             ET_TABLE_DATA,
                             true, third_allocated_element_offset,
                             false, 0, 0);
    assert_element_on_offset(file_data_pointer,
                             second_allocated_element_offset,
                             MIN_ELEMENT_SIZE,
                             ET_TABLE_METADATA,
                             false, 0,
                             false, 0, first_allocated_element_offset);
    assert_element_on_offset(file_data_pointer,
                             third_allocated_element_offset,
                             MIN_ELEMENT_SIZE,
                             ET_TABLE_DATA,
                             false, 0,
                             true, first_allocated_element_offset, second_allocated_element_offset);
    munmap_file(file_data_pointer, file_size);

    delete_element(first_allocated_element_offset);
    // deleted -> table_metadata -> table_data

    file_size = get_file_size();
    mmap_file(&file_data_pointer, 0, file_size);
    file_header = (struct FileHeader *) file_data_pointer;
    assert(file_header->last_element_offset == third_allocated_element_offset);
    assert(file_header->has_table_data_elements);
    assert(file_header->last_table_data_element_offset == third_allocated_element_offset);
    assert(file_header->has_table_metadata_elements);
    assert(file_header->last_table_metadata_element_offset == second_allocated_element_offset);
    assert_element_on_offset(file_data_pointer,
                             first_allocated_element_offset,
                             MIN_ELEMENT_SIZE,
                             ET_DELETED,
                             false, 0,
                             false, 0, 0);
    assert_element_on_offset(file_data_pointer,
                             second_allocated_element_offset,
                             MIN_ELEMENT_SIZE,
                             ET_TABLE_METADATA,
                             false, 0,
                             false, 0, first_allocated_element_offset);
    assert_element_on_offset(file_data_pointer,
                             third_allocated_element_offset,
                             MIN_ELEMENT_SIZE,
                             ET_TABLE_DATA,
                             false, 0,
                             false, first_allocated_element_offset, second_allocated_element_offset);

    munmap_file(file_data_pointer, file_size);
    close_file();
    delete_file(TEST_FILE_LOCATION);
}

void allocator_test_with_multiple_insertions() {
    open_file(TEST_FILE_LOCATION);
    uint64_t first_allocated_element_offset;
    allocate_element(100, ET_TABLE_DATA, &first_allocated_element_offset);
    uint64_t second_allocated_element_offset;
    allocate_element(200, ET_TABLE_METADATA, &second_allocated_element_offset);
    uint64_t third_allocated_element_offset;
    allocate_element(300, ET_TABLE_DATA, &third_allocated_element_offset);
    uint64_t fourth_allocated_element_offset;
    allocate_element(400, ET_TABLE_DATA, &fourth_allocated_element_offset);
    uint64_t fifth_allocated_element_offset;
    allocate_element(500, ET_TABLE_METADATA, &fifth_allocated_element_offset);
    // table_data -> table_metadata -> table_data -> table_data -> table_metadata

    void *file_data_pointer;
    uint64_t file_size = get_file_size();
    mmap_file(&file_data_pointer, 0, file_size);
    struct FileHeader *file_header = (struct FileHeader *) file_data_pointer;
    assert(file_header->last_element_offset == fifth_allocated_element_offset);
    assert(file_header->has_table_data_elements);
    assert(file_header->last_table_data_element_offset == fourth_allocated_element_offset);
    assert(file_header->has_table_metadata_elements);
    assert(file_header->last_table_metadata_element_offset == fifth_allocated_element_offset);
    assert_element_on_offset(file_data_pointer,
                             FIRST_ELEMENT_OFFSET,
                             MIN_ELEMENT_SIZE,
                             ET_TABLE_DATA,
                             true, third_allocated_element_offset,
                             false, 0, 0);
    assert_element_on_offset(file_data_pointer,
                             second_allocated_element_offset,
                             MIN_ELEMENT_SIZE,
                             ET_TABLE_METADATA,
                             true, fifth_allocated_element_offset,
                             false, 0, first_allocated_element_offset);
    assert_element_on_offset(file_data_pointer,
                             third_allocated_element_offset,
                             MIN_ELEMENT_SIZE,
                             ET_TABLE_DATA,
                             true, fourth_allocated_element_offset,
                             true, first_allocated_element_offset, second_allocated_element_offset);
    assert_element_on_offset(file_data_pointer,
                             fourth_allocated_element_offset,
                             MIN_ELEMENT_SIZE,
                             ET_TABLE_DATA,
                             false, 0,
                             true, third_allocated_element_offset, third_allocated_element_offset);
    assert_element_on_offset(file_data_pointer,
                             fifth_allocated_element_offset,
                             MIN_ELEMENT_SIZE,
                             ET_TABLE_METADATA,
                             false, 0,
                             true, second_allocated_element_offset, fourth_allocated_element_offset);

    munmap_file(file_data_pointer, file_size);
    close_file();
    delete_file(TEST_FILE_LOCATION);
}

void allocator_test_with_multiple_insertions_and_deletions() {
    open_file(TEST_FILE_LOCATION);
    uint64_t first_allocated_element_offset;
    allocate_element(100, ET_TABLE_DATA, &first_allocated_element_offset);
    uint64_t second_allocated_element_offset;
    allocate_element(200, ET_TABLE_METADATA, &second_allocated_element_offset);
    uint64_t third_allocated_element_offset;
    allocate_element(300, ET_TABLE_DATA, &third_allocated_element_offset);
    uint64_t fourth_allocated_element_offset;
    allocate_element(400, ET_TABLE_DATA, &fourth_allocated_element_offset);
    uint64_t fifth_allocated_element_offset;
    allocate_element(500, ET_TABLE_METADATA, &fifth_allocated_element_offset);
    // table_data -> table_metadata -> table_data -> table_data -> table_metadata

    delete_element(first_allocated_element_offset);
    delete_element(third_allocated_element_offset);
    delete_element(fifth_allocated_element_offset);
    // deleted -> table_metadata -> deleted -> table_data -> deleted

    void *file_data_pointer;
    uint64_t file_size = get_file_size();
    mmap_file(&file_data_pointer, 0, file_size);
    struct FileHeader *file_header = (struct FileHeader *) file_data_pointer;
    assert(file_header->last_element_offset == fifth_allocated_element_offset);
    assert(file_header->has_table_data_elements);
    assert(file_header->last_table_data_element_offset == fourth_allocated_element_offset);
    assert(file_header->has_table_metadata_elements);
    assert(file_header->last_table_metadata_element_offset == second_allocated_element_offset);
    assert(file_header->has_deleted_elements);
    assert(file_header->last_deleted_element_offset == fifth_allocated_element_offset);
    assert_element_on_offset(file_data_pointer,
                             FIRST_ELEMENT_OFFSET,
                             MIN_ELEMENT_SIZE,
                             ET_DELETED,
                             true, third_allocated_element_offset,
                             false, 0, 0);
    assert_element_on_offset(file_data_pointer,
                             second_allocated_element_offset,
                             MIN_ELEMENT_SIZE,
                             ET_TABLE_METADATA,
                             false, 0,
                             false, 0, first_allocated_element_offset);
    assert_element_on_offset(file_data_pointer,
                             third_allocated_element_offset,
                             MIN_ELEMENT_SIZE,
                             ET_DELETED,
                             true, fifth_allocated_element_offset,
                             true, first_allocated_element_offset, second_allocated_element_offset);
    assert_element_on_offset(file_data_pointer,
                             fourth_allocated_element_offset,
                             MIN_ELEMENT_SIZE,
                             ET_TABLE_DATA,
                             false, 0,
                             false, 0, third_allocated_element_offset);
    assert_element_on_offset(file_data_pointer,
                             fifth_allocated_element_offset,
                             MIN_ELEMENT_SIZE,
                             ET_DELETED,
                             false, 0,
                             true, third_allocated_element_offset, fourth_allocated_element_offset);

    munmap_file(file_data_pointer, file_size);
    close_file();
    delete_file(TEST_FILE_LOCATION);
}

void allocator_test_with_insertion_in_deleted_space() {
    open_file(TEST_FILE_LOCATION);
    uint64_t first_allocated_element_offset;
    allocate_element(MIN_ELEMENT_SIZE * 2, ET_TABLE_DATA, &first_allocated_element_offset);
    uint64_t second_allocated_element_offset;
    allocate_element(MIN_ELEMENT_SIZE * 3, ET_TABLE_DATA, &second_allocated_element_offset);
    uint64_t third_allocated_element_offset;
    allocate_element(MIN_ELEMENT_SIZE * 2, ET_TABLE_DATA, &third_allocated_element_offset);
    delete_element(second_allocated_element_offset);
    uint64_t fourth_allocated_element_offset;
    allocate_element(MIN_ELEMENT_SIZE * 1, ET_TABLE_METADATA, &fourth_allocated_element_offset);

    void *file_data_pointer;
    uint64_t file_size = get_file_size();
    mmap_file(&file_data_pointer, 0, file_size);
    struct FileHeader *file_header = (struct FileHeader *) file_data_pointer;
    assert(file_header->last_element_offset == third_allocated_element_offset);
    assert(file_header->has_table_data_elements);
    assert(file_header->last_table_data_element_offset == third_allocated_element_offset);
    assert(file_header->has_table_metadata_elements);
    assert(file_header->last_table_metadata_element_offset == fourth_allocated_element_offset);
    assert(fourth_allocated_element_offset == second_allocated_element_offset);
    assert_element_on_offset(file_data_pointer,
                             first_allocated_element_offset,
                             MIN_ELEMENT_SIZE * 2 + sizeof(struct ElementHeader),
                             ET_TABLE_DATA,
                             true, third_allocated_element_offset,
                             false, 0, 0);
    assert_element_on_offset(file_data_pointer,
                             second_allocated_element_offset,
                             MIN_ELEMENT_SIZE * 1 + sizeof(struct ElementHeader),
                             ET_TABLE_METADATA,
                             false, 0,
                             false, 0, first_allocated_element_offset);
    struct ElementHeader *element_header_of_last_inserted_element = (struct ElementHeader *) (
        ((char *) file_data_pointer) + fourth_allocated_element_offset);
    uint64_t expected_new_element_offset =
        element_header_of_last_inserted_element->element_size + fourth_allocated_element_offset;
    assert_element_on_offset(file_data_pointer,
                             expected_new_element_offset,
                             MIN_ELEMENT_SIZE * 3 - element_header_of_last_inserted_element->element_size +
                             sizeof(struct ElementHeader),
                             ET_DELETED,
                             false, 0,
                             false, 0, second_allocated_element_offset);
    assert_element_on_offset(file_data_pointer,
                             third_allocated_element_offset,
                             MIN_ELEMENT_SIZE * 2 + sizeof(struct ElementHeader),
                             ET_TABLE_DATA,
                             false, 0,
                             true, first_allocated_element_offset, expected_new_element_offset);

    munmap_file(file_data_pointer, file_size);
    close_file();
    delete_file(TEST_FILE_LOCATION);
}

void allocator_test_on_merging_deleted1() {
    open_file(TEST_FILE_LOCATION);
    uint64_t first_allocated_element_offset;
    allocate_element(MIN_ELEMENT_SIZE * 2, ET_TABLE_DATA, &first_allocated_element_offset);
    uint64_t second_allocated_element_offset;
    allocate_element(MIN_ELEMENT_SIZE * 3, ET_TABLE_DATA, &second_allocated_element_offset);
    uint64_t third_allocated_element_offset;
    allocate_element(MIN_ELEMENT_SIZE * 2, ET_TABLE_DATA, &third_allocated_element_offset);

    delete_element(second_allocated_element_offset);

    delete_element(first_allocated_element_offset);

    uint64_t file_size = get_file_size();
    void *file_data_pointer;
    mmap_file(&file_data_pointer, 0, file_size);
    struct FileHeader *file_header = (struct FileHeader *) file_data_pointer;
    assert(file_header->last_element_offset == third_allocated_element_offset);
    assert(file_header->has_table_data_elements);
    assert(file_header->last_table_data_element_offset == third_allocated_element_offset);
    assert(file_header->has_deleted_elements);
    assert(file_header->last_deleted_element_offset == first_allocated_element_offset);

    uint64_t min_element_size = MIN_ELEMENT_SIZE;
    uint64_t expected_merged_size = min_element_size * 5 + sizeof(struct ElementHeader) * 2;
    assert_element_on_offset(file_data_pointer,
                             first_allocated_element_offset,
                             expected_merged_size,
                             ET_DELETED,
                             false, 0,
                             false, 0, 0);
    assert_element_on_offset(file_data_pointer,
                             third_allocated_element_offset,
                             MIN_ELEMENT_SIZE * 2 + sizeof(struct ElementHeader),
                             ET_TABLE_DATA,
                             false, 0,
                             false, 0, first_allocated_element_offset);

    munmap_file(file_data_pointer, file_size);
    close_file();
    delete_file(TEST_FILE_LOCATION);
}


void allocator_test_on_merging_deleted2() {
    open_file(TEST_FILE_LOCATION);
    uint64_t first_allocated_element_offset;
    allocate_element(MIN_ELEMENT_SIZE * 2, ET_TABLE_DATA, &first_allocated_element_offset);
    uint64_t second_allocated_element_offset;
    allocate_element(MIN_ELEMENT_SIZE * 3, ET_TABLE_DATA, &second_allocated_element_offset);
    uint64_t third_allocated_element_offset;
    allocate_element(MIN_ELEMENT_SIZE * 2, ET_TABLE_DATA, &third_allocated_element_offset);

    delete_element(first_allocated_element_offset);

    delete_element(second_allocated_element_offset);


    uint64_t file_size = get_file_size();
    void *file_data_pointer;
    mmap_file(&file_data_pointer, 0, file_size);
    struct FileHeader *file_header = (struct FileHeader *) file_data_pointer;
    assert(file_header->last_element_offset == third_allocated_element_offset);
    assert(file_header->has_table_data_elements);
    assert(file_header->last_table_data_element_offset == third_allocated_element_offset);
    assert(file_header->has_deleted_elements);
    assert(file_header->last_deleted_element_offset == first_allocated_element_offset);

    uint64_t min_element_size = MIN_ELEMENT_SIZE;
    uint64_t expected_merged_size = min_element_size * 5 + sizeof(struct ElementHeader) * 2;
    assert_element_on_offset(file_data_pointer,
                             first_allocated_element_offset,
                             expected_merged_size,
                             ET_DELETED,
                             false, 0,
                             false, 0, 0);
    assert_element_on_offset(file_data_pointer,
                             third_allocated_element_offset,
                             MIN_ELEMENT_SIZE * 2 + sizeof(struct ElementHeader),
                             ET_TABLE_DATA,
                             false, 0,
                             false, 0, first_allocated_element_offset);

    delete_file(TEST_FILE_LOCATION);
}


int main() {
    allocator_test_single_type();
    print_separator();
    allocator_test_multiple_types();
    print_separator();
    allocator_test_with_multiple_insertions();
    print_separator();
    allocator_test_with_multiple_insertions_and_deletions();
    print_separator();
    allocator_test_with_insertion_in_deleted_space();
    print_separator();
    allocator_test_on_merging_deleted1();
    print_separator();
    allocator_test_on_merging_deleted2();

    printf("\033[0;32m");
    printf("All tests for %s passed!\n", __FILE__);
    printf("\033[0m");
}