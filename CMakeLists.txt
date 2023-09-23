cmake_minimum_required(VERSION 3.26)
project(llp_database C)

set(CMAKE_C_STANDARD 17)

add_library(llp_database
        include/db.h
        src/db/init_db.c
        src/utils/logging.c
        src/utils/logging.h
)

add_executable(test test/test.c
        include/db.h
        src/db/init_db.c
        src/utils/logging.c
        src/utils/logging.h
)