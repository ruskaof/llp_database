//
// Created by ruskaof on 23/09/23.
//

#ifndef LLP_DATABASE_PAGE_H
#define LLP_DATABASE_PAGE_H

#include <stdlib.h>
#include "../../include/table_schema.h"

#define PAGE_SIZE 65536

struct TableFieldItem {
    struct TableFieldItem *prev_item;
    size_t data_size;
    enum TableFieldDataType data_type;
    void *data;
};

struct TablePage {
    size_t items_length;
    size_t items_size;
    struct TablePage *next_table_page;
    struct TableFieldItem *last_item;
};

#endif //LLP_DATABASE_PAGE_H
