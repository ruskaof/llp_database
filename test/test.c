//
// Created by d.rusinov on 23.09.2023.
//

#include <stdio.h>
#include <string.h>

#include "../include/schema.h"
#include "../src/utils/logging.h"
#include "../src/db/file.h"

void print_table_schema(struct TableSchema *table_schema) {
    if (table_schema == NULL) {
        printf("Table schema is NULL\n");
        return;
    }

    printf("Table schema: %s\n", table_schema->name);
    printf("Columns count: %zu\n", table_schema->columns_count);
    printf("Columns:\n");

    for (size_t i = 0; i < table_schema->columns_count; i++) {
        printf("Column %zu: %s\n", i, table_schema->columns[i].name);
    }
}

int main() {
    int fd = open_file("/home/ruskaof/Desktop/testdb");


    struct TableSchema *table_schema = malloc(sizeof(struct TableSchema) + 2 * sizeof(struct TableColumn));

    strcpy(table_schema->name, "test_table4");
    table_schema->columns_count = 2;
    table_schema->columns[0].type = TD_INT64;
    strcpy(table_schema->columns[0].name, "id4");
    table_schema->columns[1].type = TD_STRING;
    strcpy(table_schema->columns[1].name, "name4");

    insert_table_schema_to_file(fd, table_schema);

    struct TableSchema *table_schema_from_file = get_table_schema_from_file(fd, "test_table4");
    print_table_schema(table_schema_from_file);


    strcpy(table_schema->name, "test_table5");

    table_schema->columns_count = 2;
    table_schema->columns[0].type = TD_INT64;
    strcpy(table_schema->columns[0].name, "id5");
    table_schema->columns[1].type = TD_STRING;
    strcpy(table_schema->columns[1].name, "name5");

    insert_table_schema_to_file(fd, table_schema);

    table_schema_from_file = get_table_schema_from_file(fd, "test_table5");
    print_table_schema(table_schema_from_file);

    delete_table_schema(fd, "test_table4");

    table_schema_from_file = get_table_schema_from_file(fd, "test_table4");
    print_table_schema(table_schema_from_file);

    table_schema_from_file = get_table_schema_from_file(fd, "test_table5");
    print_table_schema(table_schema_from_file);


    close_file(fd);
}