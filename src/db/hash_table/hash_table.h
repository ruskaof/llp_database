//
// Created by ruskaof on 24/09/23.
//

#ifndef LLP_DATABASE_HASH_TABLE_H
#define LLP_DATABASE_HASH_TABLE_H

#include "../page.h"

#include <stdlib.h>

#define TABLE_HASH_SIZE 256

struct HashTableEntry {
    u_int64_t hash;
    struct TablePage *last_table_page;
    struct HashTableEntry *next;
};

struct HashTable {
    size_t buckets_count;
    size_t entries_count;
    struct HashTableEntry *entries[];
};

void insert_or_update_page(
    struct HashTable *hash_table,
    const char *table_name,
    struct TablePage *table_page
);

struct TablePage *get_last_table_page(
    const struct HashTable *hash_table,
    const char *table_name
);

void create_hash_table(size_t size, struct HashTable *hash_table);

#endif //LLP_DATABASE_HASH_TABLE_H
