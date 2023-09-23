//
// Created by d.rusinov on 23.09.2023.
//

#include "../src/db/file/file_internal.h"

int main() {
    int fd = open_file("/home/ruskaof/Desktop/my_db");
    close_file(fd);
}