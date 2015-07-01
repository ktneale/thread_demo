#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>

#if 0 
#ifdef DEBUG_BUILD
    #define DEBUG0(C, M) debug0(C, M,__func__, __FILE__, __LINE__)
    #define DEBUG(C, M, ...) debug(C, M, ##__VA_ARGS__, __func__, __FILE__, __LINE__)
#else
    #define DEBUG0(C, M)
    #define DEBUG(C, M, ...)
#endif
#endif

#define DEBUG0(module, level, msg) do {   \
    if (debug_ ## module ## _level >= DEBUG_LEVEL_ ## level) \
        fprintf(stderr, #module ": " #level ": (" msg ") __func__, __FILE__, __LINE__);                   \
    } while (0)



//Define your module heres.  A single variable is used to track the desired debug level for each module.

int debug_QUEUE_level
int debug_MAIN_level;

#if 0 
typedef enum
{
    DEBUG_ALL = 0,
    DEBUG_INFO = 1,
    DEBUG_WARNING = 2,
    DEBUG_MAJOR = 3,
    DEBUG_CRITICAL = 4,
    DEBUG_NONE = 5
}debug_levels_t;
#endif 

typedef enum
{
    DEBUG_NONE = 0,
    DEBUG_DETAIL = 1,
    DEBUG_HIGH = 2,
    DEBUG_MEDIUM = 3,
    DEBUG_LOW = 4,
    DEBUG_CRITICAL = 5
}debug_levels_t;



void debug0(const char * module, int level, char * msg, const char * func, const char * file, const int line)
{
    if(!msg)
    {
        fprintf(stderr, "[WARNING] (Error! Bad parameter(s)) %s (%s, line %d)\n", __func__, __FILE__, __LINE__);
    }
 
    switch(level)
    {
        case DEBUG_INFO:
        {
            if(g_myapp_config.debug_level <= DEBUG_INFO)
                fprintf(stderr, "[INFO] (%s) %s (%s, line %d)\n", msg, func, file, line);
            break;
        }
        case DEBUG_WARNING:
        {
            if(g_myapp_config.debug_level <= DEBUG_WARNING)
                fprintf(stderr, "[WARNING] (%s) %s (%s, line %d)\n", msg, func, file, line);
            break;
        }
        case DEBUG_MAJOR:
        {
            if(g_myapp_config.debug_level < DEBUG_NONE)
                fprintf(stderr, "[MAJOR] (%s) %s (%s, line %d)\n", msg, func, file, line);
            break;
        }
        case DEBUG_CRITICAL:
        {
            if(g_myapp_config.debug_level < DEBUG_NONE)
                fprintf(stderr, "[CRITICAL] (%s) %s (%s, line %d)\n", msg, func, file, line);
            break;
        }
        default:
        {
            break;
        }
    }
}

void debug(int level, char * fmt, ...)
{
    if(!fmt)
    {
        fprintf(stderr, "[WARNING] (Error! Bad parameter(s)) %s (%s, line %d)\n", __func__, __FILE__, __LINE__);
    }

    int fmt_len = strlen(fmt);

    if(fmt_len <= 0 )
        return;

    //Append extended function name, source file and line number format specifiers e.g. %s (%s, %d)
    const char * ext_fmt_specifiers = "%s (%s, %d)";
    
    char * full_fmt = malloc(fmt_len + 32);

    if(!full_fmt)
        return;

    memset(full_fmt,'\0',fmt_len + 32);

    snprintf(full_fmt,fmt_len+32,"[INFO] (%s) %s\n", fmt, ext_fmt_specifiers);  //Revisit this!

    //Now full_fmt is a string containing all of the required fmt specifiers
    //e.g. variable_arg_fmt_specifiers  + ext_fmt_specifiers

    switch(level)
    {
        case DEBUG_INFO:
        {
            if(g_myapp_config.debug_level <= DEBUG_INFO)
            {
                  va_list args;
                  va_start (args, fmt);
                  vfprintf (stderr,full_fmt, args);
                  va_end (args);
            }
            break;
        }
        case DEBUG_WARNING:
        {
            if(g_myapp_config.debug_level <= DEBUG_WARNING)
                //fprintf(stderr, "[WARNING] (%s) %s (%s, line %d)\n", msg, func, file, line);
            break;
        }
        case DEBUG_MAJOR:
        {
            if(g_myapp_config.debug_level < DEBUG_NONE)
                //fprintf(stderr, "[MAJOR] (%s) %s (%s, line %d)\n", msg, func, file, line);
            break;
        }
        case DEBUG_CRITICAL:
        {
            if(g_myapp_config.debug_level < DEBUG_NONE)
                //fprintf(stderr, "[CRITICAL] (%s) %s (%s, line %d)\n", msg, func, file, line);
            break;
        }
        default:
        {
            break;
        }

        if(full_fmt) free(full_fmt);
    }
}
