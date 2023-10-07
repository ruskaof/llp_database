//
// Created by ruskaof on 2/10/23.
//

#ifndef LLP_DATABASE_ELEMENT_ALLOCATOR_H
#define LLP_DATABASE_ELEMENT_ALLOCATOR_H

#include "element.h"

#include <stdlib.h>
#include <stdint.h>

int allocate_element(uint64_t requested_element_size, enum ElementType element_type, uint64_t *element_offset);

int delete_element(uint64_t element_offset);

#endif //LLP_DATABASE_ELEMENT_ALLOCATOR_H
