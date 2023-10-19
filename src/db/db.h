//
// Created by ruskaof on 10/19/23.
//

#ifndef LLP_DATABASE_DB_H
#define LLP_DATABASE_DB_H

#include <stdint.h>
#include <stdbool.h>

#define PAGE_SIZE 65536
#define FIRST_METADATA_PAGE_OFFSET 0

struct PageHeader {
    bool has_next_page;
    uint64_t next_page_offset;
};

struct PageHeader *mmap_page_header(uint64_t page_offset);

struct PageHeader *munmap_page_header(struct PageHeader *page_header);

bool has_first_page();

uint64_t allocate_page();

#endif //LLP_DATABASE_DB_H
