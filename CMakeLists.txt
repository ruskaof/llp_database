cmake_minimum_required(VERSION 3.22.1)
project(llp_database C)

set(CMAKE_C_STANDARD 17)

add_library(llp_database
        src/utils/logging.c
        src/utils/logging.h
        include/data_operations.h
        include/table.h
        include/table_operations.h
        include/db.h
        src/db/db.c
        src/db/db.h
        src/db/table_operations.c
        src/db/elements.h
)
