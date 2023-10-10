//
// Created by ruskaof on 10/10/23.
//

#ifndef LLP_DATABASE_COMPARATOR_H
#define LLP_DATABASE_COMPARATOR_H

#include "../../include/table.h"

bool first_value_is_greater_than_second(enum TableDatatype type,
                                        uint64_t value1_size,
                                        uint64_t value2_size,
                                        void *value1,
                                        void *value2);

bool first_value_is_less_than_second(enum TableDatatype type,
                                     uint64_t value1_size,
                                     uint64_t value2_size,
                                     void *value1,
                                     void *value2);

bool first_value_is_equal_to_second(enum TableDatatype type,
                                    uint64_t value1_size,
                                    uint64_t value2_size,
                                    void *value1,
                                    void *value2);

#endif //LLP_DATABASE_COMPARATOR_H
