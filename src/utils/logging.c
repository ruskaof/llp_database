//
// Created by d.rusinov on 23.09.2023.
//

#include "logging.h"

#include <stdio.h>
#include <time.h>
#include <string.h>
#include <stdarg.h>

enum LogLevel program_log_level = LL_DEBUG;

char *getLogLevelName(enum LogLevel log_level) {
    switch (log_level) {
        case LL_DEBUG:
            return "DEBUG";
        case LL_INFO:
            return "INFO";
        case LL_WARN:
            return "WARN";
        case LL_ERROR:
            return "ERROR";
    }
    return "UNDEFINED_LOG_LEVEL";
}

char *calc_now_str() {
    time_t now;
    time(&now);
    char *time_str = ctime(&now);
    time_str[strlen(time_str) - 1] = '\0';
    return time_str;
}

void logger(enum LogLevel log_level, const char *tag, const char *message, ...) {
    if (program_log_level <= log_level) {
        va_list args;
        va_start(args, message);

        char *now = calc_now_str();

        printf("%s - %s [%s]: ", now, getLogLevelName(log_level), tag);
        vprintf(message, args);
        printf("\n");
    }
}
