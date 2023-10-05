cmake_minimum_required(VERSION 3.26)
project(llp_database C)

set(CMAKE_C_STANDARD 17)

add_library(llp_database
        src/utils/logging.c
        src/utils/logging.h
        src/db/file.h
        src/db/file.c
        src/db/page.c
        src/db/element.h
        src/db/data_operations.c
        include/data_operations.h
        include/table.h
        include/table_operations.h
        src/db/allocator.c
        src/db/allocator.h
        src/db/table_metadata.h
        src/db/table_operations.c
)

add_executable(test test/test.c
        src/utils/logging.c
        src/utils/logging.h
        src/db/file.h
        src/db/file.c
        src/db/page.c
        src/db/element.h
        src/db/data_operations.c
        include/data_operations.h
        include/table.h
        include/table_operations.h
        src/db/allocator.c
        src/db/allocator.h
        src/db/table_metadata.h
        src/db/table_operations.c
)
