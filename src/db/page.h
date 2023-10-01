//
// Created by ruskaof on 1/10/23.
//

#ifndef LLP_DATABASE_PAGE_H
#define LLP_DATABASE_PAGE_H

#include <stdbool.h>

#define TABLE_SCHEMA_PAGE_SIZE 65536
#define TABLE_SCHEMA_PAGE_OFFSET 0
#define TABLE_DATA_FIRST_PAGE_OFFSET TABLE_SCHEMA_PAGE_SIZE

struct PageHeader {
    bool has_elements;
};

#endif //LLP_DATABASE_PAGE_H
