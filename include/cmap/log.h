#ifndef __HM_LOG_H_
#define __HM_LOG_H_

typedef enum {
    LOG_LEVEL_NOLOG,
    LOG_LEVEL_INFO,
    LOG_LEVEL_WARNING,
    LOG_LEVEL_ERROR,
    LOG_LEVEL_DEBUG
} log_lv_t;

extern log_lv_t global_log_level;

void logger(log_lv_t lv, const char* file, const char* function, int line, const char* fmt, ...);

#define HM_LOG(lv, fmt, ...)                                                  \
    do {                                                                      \
        if (lv <= global_log_level) {                                         \
            logger(lv, __FILE__, __FUNCTION__, __LINE__, fmt, ##__VA_ARGS__); \
        }                                                                     \
    } while (0)

#endif
