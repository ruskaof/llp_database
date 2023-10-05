////
//// Created by ruskaof on 2/10/23.
////
//
//#include "../../include/table_operations.h"
//#include "../utils/logging.h"
//#include "table_metadata.h"
//#include "allocator.h"
//#include "file.h"
//
//#include <string.h>
//
//struct TableMetadataLLNode {
//    bool is_last;
//    struct TableMetadata table_metadata;
//};
//
//uint64_t get_table_metadata_size(struct TableMetadata *table_metadata) {
//    return sizeof(struct TableMetadata) + table_metadata->columns_count * sizeof(struct TableColumn);
//}
//
//uint64_t get_table_metadata_ll_node_size(struct TableMetadataLLNode *table_metadata_ll_node) {
//    return sizeof(struct TableMetadataLLNode) + get_table_metadata_size(&table_metadata_ll_node->table_metadata);
//}
//
//int find_table_metadata_on_page(void *page_data_pointer, char *table_name, uint64_t *table_metadata_offset,
//                                uint64_t *table_metadata_size) {
//    struct ElementHeader *page_header = (struct ElementHeader *) page_data_pointer;
//
//    if (!page_header->has_elements) {
//        return -1;
//    }
//
//    struct TableMetadataLLNode *table_metadata_ll_node = (struct TableMetadataLLNode *)
//        ((char *) page_data_pointer + sizeof(struct ElementHeader) + sizeof(struct TableMetadataPageSubHeader));
//
//    while (!table_metadata_ll_node->is_last) {
//        if (strcmp(table_metadata_ll_node->table_metadata.name, table_name) == 0) {
//            *table_metadata_offset = (uint64_t) ((char *) table_metadata_ll_node - (char *) page_data_pointer);
//            *table_metadata_size = get_table_metadata_ll_node_size(table_metadata_ll_node);
//            return 0;
//        }
//
//        table_metadata_ll_node = (struct TableMetadataLLNode *)
//            ((char *) table_metadata_ll_node + get_table_metadata_ll_node_size(table_metadata_ll_node));
//    }
//
//    if (strcmp(table_metadata_ll_node->table_metadata.name, table_name) == 0) {
//        *table_metadata_offset = (uint64_t) ((char *) table_metadata_ll_node - (char *) page_data_pointer);
//        *table_metadata_size = get_table_metadata_ll_node_size(table_metadata_ll_node);
//        return 0;
//    }
//
//    return -1;
//}
//
//int find_table_metadata(int fd, char *table_name, uint64_t *table_metadata_offset, uint64_t *table_metadata_size) {
//    uint64_t file_size = get_file_size(fd);
//
//    void *file_data_pointer;
//    int mmap_res = mmap_file(fd, &file_data_pointer, 0, file_size);
//    if (mmap_res != 0) {
//        logger(LL_ERROR, __func__, "Failed to mmap file");
//        return -1;
//    }
//
//    uint64_t current_offset = 0;
//
//    while (current_offset < file_size) {
//        struct ElementHeader *page_header = (struct ElementHeader *) ((char *) file_data_pointer + current_offset);
//
//        if (page_header->page_type == ET_TABLE_METADATA) {
//            int find_table_metadata_res = find_table_metadata_on_page(
//                (char *) file_data_pointer + current_offset,
//                table_name,
//                table_metadata_offset,
//                table_metadata_size
//            );
//
//            if (find_table_metadata_res == 0) {
//                *table_metadata_offset += current_offset;
//
//                int munmap_res = munmap_file(file_data_pointer, file_size);
//                if (munmap_res != 0) {
//                    logger(LL_ERROR, __func__, "Failed to munmap file");
//                    return -1;
//                }
//
//                return 0;
//            }
//
//        }
//        current_offset += page_header->page_size;
//    }
//
//    int munmap_res = munmap_file(file_data_pointer, file_size);
//    if (munmap_res != 0) {
//        logger(LL_ERROR, __func__, "Failed to munmap file");
//        return -1;
//    }
//
//    return -1;
//}
//
//int insert_table_metadata_in_page(void *page_data_pointer, struct TableMetadata *table_metadata) {
//    logger(LL_DEBUG, __func__, "Inserting table metadata in page");
//
//    struct ElementHeader *page_header = (struct ElementHeader *) page_data_pointer;
//
//    if (!page_header->has_elements) {
//        struct TableMetadataLLNode *table_metadata_ll_node = (struct TableMetadataLLNode *)
//            ((char *) page_data_pointer + sizeof(struct ElementHeader) + sizeof(struct TableMetadataPageSubHeader));
//        table_metadata_ll_node->is_last = true;
//        table_metadata_ll_node->table_metadata = *table_metadata;
//        page_header->has_elements = true;
//
//        logger(LL_DEBUG, __func__, "Inserted table metadata in page to offset from page start %ld",
//               (char *) table_metadata_ll_node - (char *) page_data_pointer);
//
//        return 0;
//    }
//
//    struct TableMetadataLLNode *table_metadata_ll_node = (struct TableMetadataLLNode *)
//        ((char *) page_data_pointer + sizeof(struct ElementHeader) + sizeof(struct TableMetadataPageSubHeader));
//
//    while (!table_metadata_ll_node->is_last) {
//        table_metadata_ll_node = (struct TableMetadataLLNode *)
//            ((char *) table_metadata_ll_node + get_table_metadata_ll_node_size(table_metadata_ll_node));
//    }
//
//    struct TableMetadataLLNode *new_table_metadata_ll_node = (struct TableMetadataLLNode *)
//        ((char *) table_metadata_ll_node + get_table_metadata_ll_node_size(table_metadata_ll_node));
//    new_table_metadata_ll_node->is_last = true;
//    new_table_metadata_ll_node->table_metadata = *table_metadata;
//    table_metadata_ll_node->is_last = false;
//
//    logger(LL_DEBUG, __func__, "Inserted table metadata in page to offset from page start %ld",
//           (char *) new_table_metadata_ll_node - (char *) page_data_pointer);
//
//    return 0;
//}
//
//int operation_create_table(int fd, char *table_name, struct TableColumn *columns, uint64_t columns_count) {
//    logger(LL_DEBUG, __func__, "Creating table %s", table_name);
//
//    uint64_t table_data_page_offset;
//    uint64_t table_data_page_size;
//    int table_data_page_alloc_res = allocate_element(fd,
//                                                  ALLOC_SIZE,
//                                                  ET_TABLE_DATA,
//                                                  &table_data_page_offset,
//                                                  &table_data_page_size);
//    if (table_data_page_alloc_res != 0) {
//        logger(LL_ERROR, __func__, "Failed to allocate page for table data");
//        return -1;
//    }
//
//    struct TableMetadata *table_metadata = malloc(
//        sizeof(struct TableMetadata) + columns_count * sizeof(struct TableColumn));
//    strcpy(table_metadata->name, table_name);
//    table_metadata->first_page_offset = table_data_page_offset;
//    table_metadata->columns_count = columns_count;
//    memcpy(table_metadata->columns, columns, columns_count * sizeof(struct TableColumn));
//
//    uint64_t table_metadata_page_offset;
//    uint64_t table_metadata_page_size;
//
//    logger(LL_DEBUG, __func__, "Finding table metadata page");
//    uint64_t file_size = get_file_size(fd);
//
//    void *file_data_pointer;
//    int mmap_res = mmap_file(fd, &file_data_pointer, 0, file_size);
//    if (mmap_res != 0) {
//        logger(LL_ERROR, __func__, "Failed to mmap file");
//        return -1;
//    }
//
//    uint64_t current_offset = 0;
//
//    while (current_offset < file_size) {
//        struct ElementHeader *page_header = (struct ElementHeader *) ((char *) file_data_pointer + current_offset);
//
//        if (page_header->page_type == ET_TABLE_METADATA) {
//            table_metadata_page_offset = current_offset;
//            table_metadata_page_size = page_header->page_size;
//
//            int munmap_res = munmap_file(file_data_pointer, file_size);
//            if (munmap_res != 0) {
//                logger(LL_ERROR, __func__, "Failed to munmap file");
//                return -1;
//            }
//
//            logger(LL_DEBUG, __func__, "Found table metadata page at offset %ld", table_metadata_page_offset);
//        }
//        current_offset += page_header->page_size;
//    }
//
//    if (table_metadata_page_find_res == 0) {
//        void *table_metadata_page_data_pointer;
//        int table_metadata_page_mmap_res = mmap_file(fd, &table_metadata_page_data_pointer, table_metadata_page_offset,
//                                                     table_metadata_page_size);
//        if (table_metadata_page_mmap_res != 0) {
//            logger(LL_ERROR, __func__, "Failed to mmap table metadata page");
//            return -1;
//        }
//
//        int insert_table_metadata_res = insert_table_metadata_in_page(table_metadata_page_data_pointer, table_metadata);
//        if (insert_table_metadata_res != 0) {
//            logger(LL_ERROR, __func__, "Failed to insert table metadata in page");
//            return -1;
//        }
//
//        free(table_metadata);
//
//        int sync_file_result = sync_file(fd);
//        if (sync_file_result != 0) {
//            logger(LL_ERROR, __func__, "Failed to sync file");
//            return -1;
//        }
//
//        int table_metadata_page_munmap_res = munmap_file(table_metadata_page_data_pointer, table_metadata_page_size);
//        if (table_metadata_page_munmap_res != 0) {
//            logger(LL_ERROR, __func__, "Failed to munmap table metadata page");
//            return -1;
//        }
//
//        logger(LL_DEBUG, __func__, "Inserted table metadata in existing table metadata page");
//        return 0;
//    }
//
//    logger(LL_DEBUG, __func__, "Table metadata page not found, creating new one");
//
//    int table_metadata_page_alloc_res = allocate_element(fd,
//                                                      TABLE_SCHEMA_PAGE_SIZE,
//                                                      ET_TABLE_METADATA,
//                                                      &table_metadata_page_offset,
//                                                      &table_metadata_page_size);
//    if (table_metadata_page_alloc_res != 0) {
//        logger(LL_ERROR, __func__, "Failed to allocate page for table metadata");
//        return -1;
//    }
//
//    void *table_metadata_page_data_pointer;
//    int table_metadata_page_mmap_res = mmap_file(fd, &table_metadata_page_data_pointer, table_metadata_page_offset,
//                                                 table_metadata_page_size);
//    if (table_metadata_page_mmap_res != 0) {
//        logger(LL_ERROR, __func__, "Failed to mmap table metadata page");
//        return -1;
//    }
//
//    insert_table_metadata_in_page(table_metadata_page_data_pointer, table_metadata);
//
//    free(table_metadata);
//
//    int sync_file_result = sync_file(fd);
//    if (sync_file_result != 0) {
//        logger(LL_ERROR, __func__, "Failed to sync file");
//        return -1;
//    }
//
//    int table_metadata_page_munmap_res = munmap_file(table_metadata_page_data_pointer, table_metadata_page_size);
//
//    if (table_metadata_page_munmap_res != 0) {
//        logger(LL_ERROR, __func__, "Failed to munmap table metadata page");
//        return -1;
//    }
//
//    logger(LL_DEBUG, __func__, "Inserted table metadata in new table metadata page");
//
//    return 0;
//}