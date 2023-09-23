//
// Created by ruskaof on 23/09/23.
//

#ifndef LLP_DATABASE_TABLE_META_H
#define LLP_DATABASE_TABLE_META_H

#define TABLE_NAME_MAX_LENGTH 63

struct TableMeta {
    char name[TABLE_NAME_MAX_LENGTH + 1];
    struct TableSchema *schema;

};

#endif //LLP_DATABASE_TABLE_META_H
