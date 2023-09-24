//
// Created by ruskaof on 24/09/23.
//

#include "hash_table.h"
#include "../../utils/logging.h"

#include <string.h>

u_int64_t calc_hash(const char *table_name, size_t size) {
    u_int64_t hash = 0;
    for (size_t i = 0; table_name[i] != '\0'; i++) {
        hash += table_name[i];
    }
    u_int64_t result = hash % size;

    logger(LL_DEBUG, __func__,
           "Calculated hash for table_name: %s, hash: %zu, buckets_count: %zu, result: %zu",
           table_name, hash, size, result);

    return result;
}

void insert_or_update_page(
    struct HashTable *hash_table,
    const char *table_name,
    struct TablePage *table_page
) {
    logger(LL_DEBUG, __func__,
           "Inserting or updating page for table_name: %s, table_page: %p",
           table_name, table_page);

    u_int64_t hash = calc_hash(table_name, hash_table->buckets_count);

    off_t hash_table_entries_offset =
        sizeof(struct HashTable) + sizeof(struct HashTableEntry *) * hash_table->buckets_count;
    struct HashTableEntry *entry = hash_table->entries[hash];
    while (entry != NULL) {
        if (entry->hash == hash) {
            logger(LL_DEBUG, __func__,
                   "Found entry with hash: %zu, table_name: %s, last_table_page: %p",
                   entry->hash, table_name, entry->last_table_page);
            entry->last_table_page = table_page;
            return;
        }
        entry = entry->next;
    }

    logger(LL_DEBUG, __func__,
           "Creating new entry with hash: %zu, table_name: %s, last_table_page: %p",
           hash, table_name, table_page);

    entry = (struct HashTableEntry *) (hash_table_entries_offset
                                       + hash_table->entries_count * sizeof(struct HashTableEntry *));
    entry->hash = hash;
    entry->last_table_page = table_page;
    entry->next = hash_table->entries[hash];
    hash_table->entries[hash] = entry;
    hash_table->entries_count++;
}

struct TablePage *get_last_table_page(const struct HashTable *hash_table, const char *table_name) {
    logger(LL_DEBUG, __func__, "Getting last table page for table_name: %s", table_name);

    u_int64_t hash = calc_hash(table_name, hash_table->buckets_count);
    struct HashTableEntry *entry = hash_table->entries[hash];
    while (entry != NULL) {
        if (entry->hash == hash) {
            logger(LL_DEBUG, __func__,
                   "Found entry with hash: %zu, table_name: %s, last_table_page: %p",
                   entry->hash, table_name, entry->last_table_page);

            return entry->last_table_page;
        }
        entry = entry->next;
    }

    logger(LL_DEBUG, __func__,
           "Not found entry with hash: %zu, table_name: %s",
           hash, table_name);
    return NULL;
}

void create_hash_table(size_t size, struct HashTable *hash_table) {
    logger(LL_DEBUG, __func__, "Creating hash table with buckets_count: %zu", size);

    for (size_t i = 0; i < size; i++) {
        hash_table->entries[i] = NULL;
    }

    logger(LL_DEBUG, __func__, "Created hash table: %p", hash_table);
}

