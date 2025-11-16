#include "pico_logs.h"
#include "pico/stdlib.h"
#include <string.h>


static int s_max_log_level = PICO_LOG_MAX_LEVEL; 

void pico_set_log_level(int level) {
    if (level >= PICO_LOG_LEVEL_ERROR && level <= PICO_LOG_LEVEL_NONE) {
        s_max_log_level = level;
    }
}

void log_style(char level, const char *tag, const char *format, ...) {

    const char *color;
    int msg_level; 
  
    switch (level) {
        case 'E': msg_level = PICO_LOG_LEVEL_ERROR; color = ANSI_COLOR_RED; break;
        case 'W': msg_level = PICO_LOG_LEVEL_WARN; color = ANSI_COLOR_YELLOW; break;
        case 'I': msg_level = PICO_LOG_LEVEL_INFO; color = ANSI_COLOR_GREEN; break;
        case 'D': msg_level = PICO_LOG_LEVEL_DEBUG; color = ANSI_COLOR_CYAN; break;
        case 'V': 
        default: msg_level = PICO_LOG_LEVEL_VERBOSE; color = ANSI_COLOR_RESET; break;
    }

   
    if (msg_level > s_max_log_level) {
        return; 
    }
    
    
    uint64_t time_ms = to_ms_since_boot(get_absolute_time());

    
    printf("%s%c (%u) %s: ", color, level, (unsigned int)time_ms, tag);

    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);

    printf(ANSI_COLOR_RESET "\n"); 
}