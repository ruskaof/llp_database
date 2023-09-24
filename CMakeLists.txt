cmake_minimum_required(VERSION 3.26)
project(llp_database C)

set(CMAKE_C_STANDARD 17)

add_library(llp_database
        src/utils/logging.c
        src/utils/logging.h
        src/db/file/file_internal.c
        src/db/file/file_internal.h
        include/table_schema.h
        src/db/page.h
        include/table_manipulation_operations.h
        src/db/table_manipulation_operations.c
        src/db/hash_table/hash_table.c
        src/db/hash_table/hash_table.h
        src/db/table_definition_operations.c
        include/table_definition_operations.h
        include/db.h
        src/db/db.c
        src/db/db.h
)

add_executable(test test/test.c
        src/utils/logging.c
        src/utils/logging.h
        src/db/file/file_internal.c
        src/db/file/file_internal.h
        include/table_schema.h
        src/db/page.h
        include/table_manipulation_operations.h
        src/db/table_manipulation_operations.c
        src/db/hash_table/hash_table.c
        src/db/hash_table/hash_table.h
        src/db/table_definition_operations.c
        include/table_definition_operations.h
        include/db.h
        src/db/db.c
        src/db/db.h
)
