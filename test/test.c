//
// Created by d.rusinov on 23.09.2023.
//

#include "../include/db.h"

int main() {
    struct DbInfo db_info = init_db_file("/Users/d.rusinov/Desktop/itmo/llp/my_db5");
    close_file(db_info.fd);
}