//
// Created by d.rusinov on 23.09.2023.
//

#ifndef LLP_DATABASE_LOGGING_H
#define LLP_DATABASE_LOGGING_H


enum LogLevel {
    LL_DEBUG = 0,
    LL_INFO = 1,
    LL_WARN = 2,
    LL_ERROR = 3,
};

void logger(enum LogLevel log_level, const char *tag, const char *message, ...);

#endif //LLP_DATABASE_LOGGING_H
