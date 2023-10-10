//
// Created by ruskaof on 23.09.2023.
//

#include "logging.h"

#include <stdio.h>
#include <time.h>
#include <string.h>
#include <stdarg.h>

enum LogLevel program_log_level = LL_WARN;

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

        if (log_level == LL_ERROR) {
            printf("\033[0;31m");
        } else if (log_level == LL_WARN) {
            printf("\033[0;33m");
        } else if (log_level == LL_INFO) {
            printf("\033[0;34m");
        }

        printf("%s - %s [%s]: ", now, getLogLevelName(log_level), tag);
        vprintf(message, args);
        printf("\n");

        if (log_level == LL_ERROR || log_level == LL_WARN || log_level == LL_INFO) {
            printf("\033[0m");
        }
    }
}
