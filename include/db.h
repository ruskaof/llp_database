//
// Created by ruskaof on 10/10/23.
//

#ifndef LLP_DATABASE_DB_H
#define LLP_DATABASE_DB_H

#include <stdint.h>

int init_db(const char *filename);

int close_db();

int delete_db_file(const char *filename);

#endif //LLP_DATABASE_DB_H
