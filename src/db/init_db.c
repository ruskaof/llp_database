//
// Created by d.rusinov on 23.09.2023.
//

#include "../utils/logging.h"
#include "../../include/db.h"

#include<unistd.h>
#include<fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>

int open_file(const char *filename) {
    mode_t mode = 0x0777;
    return open(filename, O_RDWR | O_CREAT | O_TRUNC, mode);
}


struct DbInfo generate_error_result() {
    struct DbInfo result;
    result.fd = -1;
    result.file_data_pointer = NULL;
    return result;
}

struct DbInfo init_db_file(const char *filename) {
    int fd = open_file(filename);
    if (fd == -1) {
        logger(LL_ERROR, __func__, "Could not open file.");
        return generate_error_result();
    }
    logger(LL_DEBUG, __func__, "Opened file with descriptor %d.", fd);

    struct stat stat_buffer;
    if (fstat(fd, &stat_buffer) < 0) {
        logger(LL_ERROR, __func__, "Fstat error.");
        return generate_error_result();
    }

    logger(LL_DEBUG, __func__, "Opened file st_size = %d", stat_buffer.st_size);

    void *db_file_data_pointer = mmap(NULL, stat_buffer.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

    struct DbInfo result;
    result.file_data_pointer = db_file_data_pointer;
    result.fd = fd;

    return result;
}



void close_file(int file_descriptor) {
    int close_result = close(file_descriptor);

    if (close_result != 0) {
        logger(LL_ERROR, __func__, "Could not close file with descriptor %d.", file_descriptor);
    } else {
        logger(LL_DEBUG, __func__, "Closed file with descriptor %d.", file_descriptor);
    }
}
