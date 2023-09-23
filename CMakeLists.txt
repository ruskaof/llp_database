cmake_minimum_required(VERSION 3.26)
project(llp_database C)

set(CMAKE_C_STANDARD 17)

add_library(llp_database
        include/db.h
        src/utils/logging.c
        src/utils/logging.h
        src/db/file/file_internal.c
        src/db/file/file_internal.h
        src/db/file/file_meta.h
        include/table_schema.h
        src/db/page/page.cpp
        src/db/page/page.h
        src/db/table/table_meta.h
)

add_executable(test test/test.c
        include/db.h
        src/utils/logging.c
        src/utils/logging.h
        src/db/file/file_internal.c
        src/db/file/file_internal.h
        src/db/file/file_meta.h
        include/table_schema.h
        src/db/page/page.cpp
        src/db/page/page.h
        src/db/table/table_meta.h
)
