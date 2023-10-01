cmake_minimum_required(VERSION 3.26)
project(llp_database C)

set(CMAKE_C_STANDARD 17)

add_library(llp_database
        src/utils/logging.c
        src/utils/logging.h
        src/db/file.h
        src/db/file.c
        src/db/schema.c
        include/schema.h
        src/db/page.c
        src/db/page.h
)

add_executable(test test/test.c
        src/utils/logging.c
        src/utils/logging.h
        src/db/file.h
        src/db/file.c
        src/db/schema.c
        include/schema.h
        src/db/page.c
        src/db/page.h
)
