//
// Created by ruskaof on 24/09/23.
//

#ifndef LLP_DATABASE_DB_H
#define LLP_DATABASE_DB_H

#include <stdlib.h>

struct OpenedDbInfo {
    int fd;
    size_t file_size;
    void *mmaped_file;
};

struct OpenedDbInfo init_db_in_file(const char *filename);

int close_db(struct OpenedDbInfo opened_db_info);

#endif //LLP_DATABASE_DB_H
