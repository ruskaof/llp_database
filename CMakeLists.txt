cmake_minimum_required(VERSION 3.22.1)
project(llp_database C)

set(CMAKE_C_STANDARD 17)

add_library(llp_database
        src/utils/logging.c
        src/utils/logging.h
        src/db/file.c
        src/db/element.h
        src/db/data_operations.c
        include/data_operations.h
        include/table.h
        include/table_operations.h
        src/db/element_allocator.c
        src/db/element_allocator.h
        src/db/table_metadata.h
        src/db/table_operations.c
        src/db/table_metadata.c
        src/db/table_data.h
        include/db.h
        src/db/table.c
        src/db/comparator.c
        src/db/comparator.h
)

add_executable(element_allocator_test test/element_allocator_test.c
        src/utils/logging.c
        src/utils/logging.h
        src/db/file.c
        src/db/element.h
        src/db/data_operations.c
        include/data_operations.h
        include/table.h
        include/table_operations.h
        src/db/element_allocator.c
        src/db/element_allocator.h
        src/db/table_metadata.h
        src/db/table_operations.c
        src/db/table_metadata.c
        src/db/table_data.h
        include/db.h
        src/db/table.c
        src/db/comparator.c
        src/db/comparator.h
)

add_executable(table_operations_test test/table_operations_test.c
        src/utils/logging.c
        src/utils/logging.h
        src/db/file.c
        src/db/element.h
        src/db/data_operations.c
        include/data_operations.h
        include/table.h
        include/table_operations.h
        src/db/element_allocator.c
        src/db/element_allocator.h
        src/db/table_metadata.h
        src/db/table_operations.c
        src/db/table_metadata.c
        src/db/table_data.h
        include/db.h
        src/db/table.c
        src/db/comparator.c
        src/db/comparator.h
)

add_executable(join_test test/join_test.c
        src/utils/logging.c
        src/utils/logging.h
        src/db/file.c
        src/db/element.h
        src/db/data_operations.c
        include/data_operations.h
        include/table.h
        include/table_operations.h
        src/db/element_allocator.c
        src/db/element_allocator.h
        src/db/table_metadata.h
        src/db/table_operations.c
        src/db/table_metadata.c
        src/db/table_data.h
        include/db.h
        src/db/table.c
        src/db/comparator.c
        src/db/comparator.h
)

add_executable(performance_test test/performance_test.c
        src/utils/logging.c
        src/utils/logging.h
        src/db/file.c
        src/db/element.h
        src/db/data_operations.c
        include/data_operations.h
        include/table.h
        include/table_operations.h
        src/db/element_allocator.c
        src/db/element_allocator.h
        src/db/table_metadata.h
        src/db/table_operations.c
        src/db/table_metadata.c
        src/db/table_data.h
        include/db.h
        src/db/table.c
        src/db/comparator.c
        src/db/comparator.h
)
