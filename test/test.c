//
// Created by d.rusinov on 23.09.2023.
//

#include "../src/db/db.h"
#include "../src/utils/logging.h"
#include "../include/db.h"

#include <stdio.h>

void print_test_error(const char *message) {
    printf("Test failed: %s\n", message);
}



int main() {
    struct OpenedDbInfo odi = init_db_in_file("/home/ruskaof/Desktop/my_db2");
    if (odi.fd == -1) {
        print_test_error("Failed to init db in file.");
        return -1;
    }

    int close_result = close_db(odi);

    if (close_result == -1) {
        print_test_error("Failed to close db.");
        return -1;
    }
}