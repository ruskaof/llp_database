//
// Created by ruskaof on 10/19/23.
//

#ifndef LLP_DATABASE_BALANCED_TREE_H
#define LLP_DATABASE_BALANCED_TREE_H

#include <stdint.h>

#define RB_TREE_ALLOCATED_SIZE 128

struct RbTree {
    uint64_t elements_count;
    uint64_t max_size;
};

struct RbElement {
    uint64_t offset;
    uint64_t size;
};

uint64_t get_rb_max(struct RbTree *tree); // max by deleted elements_count

void insert_rb(struct RbTree *tree, uint64_t offset, uint64_t deleted_size);

void delete_element_rb(struct RbTree *tree, uint64_t offset);

#endif //LLP_DATABASE_BALANCED_TREE_H
