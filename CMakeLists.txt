cmake_minimum_required(VERSION 3.22.1)
project(llp_database C)

set(CMAKE_C_STANDARD 17)

# set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -Werror -fstack-protector -D_FORTIFY_SOURCE=2 -fsanitize=address")

add_library(llp_database
        src/utils/logging.c
        src/utils/logging.h
        src/db/file.c
        src/db/page.c
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
        include/file.h
)

add_executable(element_allocator_test test/element_allocator_test.c
        src/utils/logging.c
        src/utils/logging.h
        src/db/file.c
        src/db/page.c
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
        include/file.h
)

add_executable(table_operations_test test/table_operations_test.c
        src/utils/logging.c
        src/utils/logging.h
        src/db/file.c
        src/db/page.c
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
        include/file.h
)

add_executable(performance_test test/performance_test.c
        src/utils/logging.c
        src/utils/logging.h
        src/db/file.c
        src/db/page.c
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
        include/file.h
)

