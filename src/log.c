#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>

#include "log.h"

log_lv_t global_log_level = LOG_LEVEL_DEBUG;

void logger(
    log_lv_t lv,
    const char* file,
    const char* function,
    int line,
    const char *fmt,
    ...
) {
    const char *log_level_str;
    switch (lv) {
        case LOG_LEVEL_INFO:    log_level_str = "[INFO]"; break;
        case LOG_LEVEL_WARNING: log_level_str = "[WARNING]"; break;
        case LOG_LEVEL_DEBUG:   log_level_str = "[DEBUG]"; break;
        default:                log_level_str = "[UNKNOWN]"; break;
    }

    va_list args;
    va_start(args, fmt);

    int size = vsnprintf(NULL, 0, fmt, args);
    va_end(args);

    char* message = (char*)malloc(size + 1); // +1 for null terminator
    if (message == NULL) {
        perror("Failed to allocate memory for log message");
        return;
    }

    va_start(args, fmt);
    vsnprintf(message, size + 1, fmt, args);
    va_end(args);

    if (lv == LOG_LEVEL_DEBUG) {
        printf("%s [%s:%d - %s]: %s\n", log_level_str, file, line, function, message);
    } else {
        printf("%s %s\n", log_level_str, message);
    }

    // Clean up
    free(message);
}
