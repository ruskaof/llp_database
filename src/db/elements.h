//
// Created by ruskaof on 10/19/23.
//

#ifndef LLP_DATABASE_ELEMENTS_H
#define LLP_DATABASE_ELEMENTS_H

#include <stdint.h>
#include <stdbool.h>

struct MetadataPageHeader {
    bool has_elements;
};

struct DataPageHeader {
    uint64_t element_count;
    uint64_t* element_offsets;
};

struct MetadataElement {
    uint64_t size;
    uint64_t has_next_element;
    uint64_t last_page_aoffset;
    uint64_t first_deleted_aoffset;
};

struct DeletedElement {
    uint64_t size;
    bool has_prev;
    uint64_t prev_size;
    uint64_t prev_aoffset;
};

#endif //LLP_DATABASE_ELEMENTS_H
