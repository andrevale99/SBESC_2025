#ifndef PICO_LOGS_H
#define PICO_LOGS_H

#include <stdio.h>
#include <stdarg.h>


#define ANSI_COLOR_RED      "\x1b[1;31m"
#define ANSI_COLOR_YELLOW   "\x1b[33m"
#define ANSI_COLOR_GREEN    "\x1b[32m"
#define ANSI_COLOR_CYAN     "\x1b[36m"
#define ANSI_COLOR_RESET    "\x1b[0m"

#define PICO_LOG_LEVEL_ERROR    0
#define PICO_LOG_LEVEL_WARN     1
#define PICO_LOG_LEVEL_INFO     2
#define PICO_LOG_LEVEL_DEBUG    3
#define PICO_LOG_LEVEL_VERBOSE  4
#define PICO_LOG_LEVEL_NONE     5 


#ifndef PICO_LOG_MAX_LEVEL
#define PICO_LOG_MAX_LEVEL PICO_LOG_LEVEL_INFO
#endif


void log_style(char level, const char *tag, const char *format, ...);

void pico_set_log_level(int level); 


#define PICO_LOGE(tag, format, ...) log_style('E', tag, format, ##__VA_ARGS__)
#define PICO_LOGW(tag, format, ...) log_style('W', tag, format, ##__VA_ARGS__)
#define PICO_LOGI(tag, format, ...) log_style('I', tag, format, ##__VA_ARGS__)
#define PICO_LOGD(tag, format, ...) log_style('D', tag, format, ##__VA_ARGS__)
#define PICO_LOGV(tag, format, ...) log_style('V', tag, format, ##__VA_ARGS__)

#endif // PICO_LOGS_H