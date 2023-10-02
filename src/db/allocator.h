//
// Created by ruskaof on 2/10/23.
//

#ifndef LLP_DATABASE_ALLOCATOR_H
#define LLP_DATABASE_ALLOCATOR_H

#include "page.h"

#include <stdlib.h>
#include <stdint.h>

int allocate_page(int fd, uint64_t min_size, enum PageType page_type, uint64_t *page_offset, uint64_t *page_size);

int delete_page(int fd, uint64_t page_offset);

#endif //LLP_DATABASE_ALLOCATOR_H
