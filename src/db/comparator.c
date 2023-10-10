//
// Created by ruskaof on 10/10/23.
//

#include <string.h>
#include "comparator.h"
#include "../utils/logging.h"

size_t min(uint64_t size, uint64_t size1);

int64_t
compare_values(enum TableDatatype type, uint64_t value1_size, uint64_t value2_size, void *value1, void *value2) {
    switch (type) {
        case TD_INT64:
            return *(int64_t *) value1 - *(int64_t *) value2;
        case TD_FLOAT64:
            return *(double *) value1 - *(double *) value2;
        case TD_STRING:
            return strncmp(value1, value2, min(value1_size, value2_size));
        case TD_BOOL:
            return *(bool *) value1 - *(bool *) value2;
    }
    return 0;
}

bool first_value_is_greater_than_second(enum TableDatatype type,
                                        uint64_t value1_size,
                                        uint64_t value2_size,
                                        void *value1,
                                        void *value2) {
    return compare_values(type, value1_size, value2_size, value1, value2) > 0;
}

bool first_value_is_less_than_second(enum TableDatatype type,
                                     uint64_t value1_size,
                                     uint64_t value2_size,
                                     void *value1,
                                     void *value2) {
    return compare_values(type, value1_size, value2_size, value1, value2) < 0;
}

bool first_value_is_equal_to_second(enum TableDatatype type,
                                    uint64_t value1_size,
                                    uint64_t value2_size,
                                    void *value1,
                                    void *value2) {
    return compare_values(type, value1_size, value2_size, value1, value2) == 0;
}

size_t min(uint64_t size, uint64_t size1) {
    return size < size1 ? size : size1;
}
