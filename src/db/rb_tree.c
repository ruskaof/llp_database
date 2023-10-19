//
// Created by ruskaof on 10/19/23.
//

#include <stdbool.h>
#include "rb_tree.h"

// rb tree on array of RbElements

uint64_t get_right_child_index(uint64_t current_index) {
    return current_index * 2 + 2;
}

uint64_t get_left_child_index(uint64_t current_index) {
    return current_index * 2 + 1;
}

uint64_t get_rb_max(struct RbTree *tree) {
    if (tree->elements_count == 0) {
        return 0;
    }

    struct RbElement *elements = (void *) tree + sizeof(struct RbTree);

    uint64_t current_index = 0;

    while (get_right_child_index(current_index) < tree->elements_count) {
        current_index = get_right_child_index(current_index);
    }

    return elements[current_index].offset;
}

void insert_rb(struct RbTree *tree, uint64_t offset, uint64_t deleted_size) {
    struct RbElement *elements = (void *) tree + sizeof(struct RbTree);

    if (tree->elements_count == 0) {
        elements[0].offset = offset;
        elements[0].size = deleted_size;
        tree->elements_count++;
        return;
    }

    uint64_t current_index = 0;

    bool inserted = false;
    while (!inserted) {
        if (deleted_size < elements[current_index].size) {
            if (get_left_child_index(current_index) < tree->elements_count) {
                current_index = get_left_child_index(current_index);
            } else {
                elements[get_left_child_index(current_index)].offset = offset;
                elements[get_left_child_index(current_index)].size = deleted_size;
                inserted = true;
            }
        } else {
            if (get_right_child_index(current_index) < tree->elements_count) {
                current_index = get_right_child_index(current_index);
            } else {
                elements[get_right_child_index(current_index)].offset = offset;
                elements[get_right_child_index(current_index)].size = deleted_size;
                inserted = true;
            }
        }
    }

    tree->elements_count++;
}

void delete_element_rb(struct RbTree *tree, uint64_t offset) {
    struct RbElement *elements = (void *) tree + sizeof(struct RbTree);

    if (tree->elements_count == 0) {
        return;
    }

    uint64_t current_index = 0;

    bool deleted = false;
    while (!deleted) {
        if (offset < elements[current_index].offset) {
            if (get_left_child_index(current_index) < tree->elements_count) {
                current_index = get_left_child_index(current_index);
            } else {
                return;
            }
        } else if (offset > elements[current_index].offset) {
            if (get_right_child_index(current_index) < tree->elements_count) {
                current_index = get_right_child_index(current_index);
            } else {
                return;
            }
        } else {
            elements[current_index].offset = 0;
            elements[current_index].size = 0;
            deleted = true;
        }
    }

    tree->elements_count--;
}
