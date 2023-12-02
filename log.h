#ifndef _LOG_H_
#define _LOG_H_
#pragma once

#include <stdio.h>

//#define MAX_LOG_LEVEL LOG_LEVEL_VERBOSE
#ifndef RELEASE_STATUS
    #define DEBUG 0
    #define RELEASE_STATUS DEBUG
#endif

#if RELEASE_STATUS == DEBUG
    #define MAX_LOG_LEVEL LOG_LEVEL_VERBOSE
#else
    // Burada logları kaldırmak için LOG_LEVEL_NONE yapılabilir.
    #define MAX_LOG_LEVEL LOG_LEVEL_VERBOSE
#endif
/*
! Önemli 
PLatform.io da serial monitorde renkli yazdırması için 
monitor_filters = direct
platformio.ini dosyasına eklenmelidir.
*/

//#define LOG_HEADER_ENABLE

#ifdef LOG_HEADER_ENABLE
    #define _LOG_HEADER 
    #define _LOG_HEADER_STATEMENT pdTICKS_TO_MS(xTaskGetTickCount())
    #define _LOG_FATAL_HEADER_STATEMENT() printf(_LOG_FATAL_MS_FORMAT("[%d]"), _LOG_HEADER_STATEMENT)
    #define _LOG_ERROR_HEADER_STATEMENT() printf(_LOG_ERROR_MS_FORMAT("[%d]"), _LOG_HEADER_STATEMENT)
    #define _LOG_WARNING_HEADER_STATEMENT() printf(_LOG_WARNING_MS_FORMAT("[%d]"), _LOG_HEADER_STATEMENT)
    #define _LOG_INFO_HEADER_STATEMENT() printf(_LOG_INFO_MS_FORMAT("[%d]"), _LOG_HEADER_STATEMENT)
    #define _LOG_DEBUG_HEADER_STATEMENT() printf(_LOG_DEBUG_MS_FORMAT("[%d]"), _LOG_HEADER_STATEMENT)
    #define _LOG_TRACE_HEADER_STATEMENT() printf(_LOG_TRACE_MS_FORMAT("[%d]"), _LOG_HEADER_STATEMENT)
    #define _LOG_VERBOSE_HEADER_STATEMENT() printf(_LOG_VERBOSE_MS_FORMAT("[%d]"), _LOG_HEADER_STATEMENT)
#endif

#define MESSAGE_TYPE_ERROR "[ERROR]"
#define MESSAGE_TYPE_WARNING "[WARNING]"
#define MESSAGE_TYPE_INFO "[INFO]"
#define MESSAGE_TYPE_DEBUG "[DEBUG]"
#define MESSAGE_TYPE_TRACE "[TRACE]"
#define MESSAGE_TYPE_FATAL "[FATAL]"
#define MESSAGE_TYPE_VERBOSE "[VERBOSE]"

#define LOG_LEVEL_NONE 0
#define LOG_LEVEL_FATAL 1
#define LOG_LEVEL_ERROR 2
#define LOG_LEVEL_WARNING 3
#define LOG_LEVEL_INFO 4
#define LOG_LEVEL_DEBUG 5
#define LOG_LEVEL_TRACE 6
#define LOG_LEVEL_VERBOSE 7

#define LOG_LEVEL_MAX 99
#define LOG_LEVEL_MIN LOG_LEVEL_ERROR


#if MAX_LOG_LEVEL == LOG_LEVEL_VERBOSE || MAX_LOG_LEVEL == LOG_LEVEL_MAX
    #define PRINT_FATAL
    #define PRINT_ERROR
    #define PRINT_WARNING
    #define PRINT_INFO
    #define PRINT_DEBUG
    #define PRINT_TRACE
    #define PRINT_VERBOSE
#elif MAX_LOG_LEVEL == LOG_LEVEL_TRACE
    #define PRINT_FATAL
    #define PRINT_ERROR
    #define PRINT_WARNING
    #define PRINT_INFO
    #define PRINT_DEBUG
    #define PRINT_TRACE
#elif MAX_LOG_LEVEL == LOG_LEVEL_DEBUG
    #define PRINT_FATAL
    #define PRINT_ERROR
    #define PRINT_WARNING
    #define PRINT_INFO
    #define PRINT_DEBUG
#elif MAX_LOG_LEVEL == LOG_LEVEL_INFO
    #define PRINT_FATAL
    #define PRINT_ERROR
    #define PRINT_WARNING
    #define PRINT_INFO
#elif MAX_LOG_LEVEL == LOG_LEVEL_WARNING
    #define PRINT_FATAL
    #define PRINT_ERROR
    #define PRINT_WARNING
#elif MAX_LOG_LEVEL == LOG_LEVEL_ERROR || MAX_LOG_LEVEL == LOG_LEVEL_MIN
    #define PRINT_FATAL
    #define PRINT_ERROR
#elif MAX_LOG_LEVEL == LOG_LEVEL_FATAL
    #define PRINT_FATAL
#elif MAX_LOG_LEVEL == LOG_LEVEL_NONE

#endif

#define RED   "\x1B[0;31m"
#define GREEN   "\x1B[0;32m"
#define YELLOW   "\x1B[0;33m"
#define BLUE   "\x1B[0;34m"
#define MAGENTA   "\x1B[0;35m"
#define CYAN   "\x1B[0;36m"
#define WHITE   "\x1B[0;37m"
#define RESET "\x1B[0m"

#define REDBOLD   "\x1B[1;31m"
#define GREENBOLD   "\x1B[1;32m"
#define YELLOWBOLD   "\x1B[1;33m"
#define BLUEBOLD   "\x1B[1;34m"
#define MAGENTABOLD   "\x1B[1;35m"
#define CYANBOLD   "\x1B[1;36m"
#define WHITEBOLD   "\x1B[1;37m"

#define BLACKBACKGROUND "\e[40m"
#define REDBACKGROUND "\e[41m"
#define GREENBACKGROUND "\e[42m"
#define YELLOWBACKGROUND "\e[43m"
#define BLUEBACKGROUND "\e[44m"
#define MAGENTABACKGROUND "\e[45m"
#define CYANBACKGROUND "\e[46m"
#define WHITEBACKGROUND "\e[47m"

#define WHITE_ON_RED "\e[37;41m"
#define WHITE_ON_GREEN "\e[37;42m"
#define BLACK_ON_YELLOW "\e[30;43m"
#define BLACK_ON_WHITE "\e[30;47m"
#define BLACK_ON_CYAN "\e[30;46m"
#define BLACK_ON_MAGENTA "\e[30;45m"
#define WHITE_ON_BLUE "\e[37;44m"
#define RED_ON_WHITE "\e[31;47m"
#define GREEN_ON_WHITE "\e[32;47m"
#define BLUE_ON_WHITE "\e[34;47m"
#define BLACK_ON_WHITE "\e[30;47m"

#define ERROR_COLOR REDBOLD
#define WARNING_COLOR YELLOWBOLD
#define INFO_COLOR GREENBOLD
#define DEBUG_COLOR CYANBOLD
#define TRACE_COLOR MAGENTA
#define FATAL_COLOR RED_ON_WHITE
#define VERBOSE_COLOR WHITEBOLD

#define _LOG_ERROR_FORMAT(format) ERROR_COLOR MESSAGE_TYPE_ERROR format RESET "\n"
#define _LOG_WARNING_FORMAT(format) WARNING_COLOR MESSAGE_TYPE_WARNING format RESET "\n"
#define _LOG_INFO_FORMAT(format) INFO_COLOR MESSAGE_TYPE_INFO format RESET "\n"
#define _LOG_DEBUG_FORMAT(format) DEBUG_COLOR MESSAGE_TYPE_DEBUG format RESET "\n"
#define _LOG_TRACE_FORMAT(format) TRACE_COLOR MESSAGE_TYPE_TRACE format RESET "\n"
#define _LOG_FATAL_FORMAT(format) FATAL_COLOR MESSAGE_TYPE_FATAL format RESET "\n"
#define _LOG_VERBOSE_FORMAT(format) VERBOSE_COLOR MESSAGE_TYPE_VERBOSE format RESET "\n"

#define _LOG_ERROR_MS_FORMAT(format) ERROR_COLOR format RESET
#define _LOG_WARNING_MS_FORMAT(format) WARNING_COLOR format RESET
#define _LOG_INFO_MS_FORMAT(format) INFO_COLOR format RESET
#define _LOG_DEBUG_MS_FORMAT(format) DEBUG_COLOR format RESET
#define _LOG_TRACE_MS_FORMAT(format) TRACE_COLOR format RESET
#define _LOG_FATAL_MS_FORMAT(format) FATAL_COLOR format RESET
#define _LOG_VERBOSE_MS_FORMAT(format) VERBOSE_COLOR format RESET


#define _LOG_ERROR1(format, ...) printf(_LOG_ERROR_FORMAT(format), __VA_ARGS__)
#define _LOG_WARNING1(format, ...) printf(_LOG_WARNING_FORMAT(format), __VA_ARGS__)
#define _LOG_INFO1(format, ...) printf(_LOG_INFO_FORMAT(format), __VA_ARGS__)
#define _LOG_DEBUG1(format, ...) printf(_LOG_DEBUG_FORMAT(format), __VA_ARGS__)
#define _LOG_TRACE1(format, ...) printf(_LOG_TRACE_FORMAT(format), __VA_ARGS__)
#define _LOG_FATAL1(format, ...) printf(_LOG_FATAL_FORMAT(format), __VA_ARGS__)
#define _LOG_VERBOSE1(format, ...) printf(_LOG_VERBOSE_FORMAT(format), __VA_ARGS__)

#define _LOG_ERROR2(format) printf(_LOG_ERROR_FORMAT(format))
#define _LOG_WARNING2(format) printf(_LOG_WARNING_FORMAT(format))
#define _LOG_INFO2(format) printf(_LOG_INFO_FORMAT(format))
#define _LOG_DEBUG2(format) printf(_LOG_DEBUG_FORMAT(format))
#define _LOG_TRACE2(format) printf(_LOG_TRACE_FORMAT(format))
#define _LOG_FATAL2(format) printf(_LOG_FATAL_FORMAT(format))
#define _LOG_VERBOSE2(format) printf(_LOG_VERBOSE_FORMAT(format))



#define _GET_MACRO(_1,_2,NAME,...) NAME

#ifdef PRINT_ERROR
    #ifdef _LOG_HEADER
        #define LOG_ERROR(...) _LOG_ERROR_HEADER_STATEMENT(); \
        _GET_MACRO(__VA_ARGS__, _LOG_ERROR1, _LOG_ERROR2)(__VA_ARGS__)
    #else
        #define LOG_ERROR(...) _GET_MACRO(__VA_ARGS__, _LOG_ERROR1, _LOG_ERROR2)(__VA_ARGS__)
    #endif
#else
    #define LOG_ERROR(...) do {} while (0)
#endif

#ifdef PRINT_WARNING
    #ifdef _LOG_HEADER
        #define LOG_WARNING(...) _LOG_WARNING_HEADER_STATEMENT(); \
        _GET_MACRO(__VA_ARGS__, _LOG_WARNING1, _LOG_WARNING2)(__VA_ARGS__)
    #else
        #define LOG_WARNING(...) _GET_MACRO(__VA_ARGS__, _LOG_WARNING1, _LOG_WARNING2)(__VA_ARGS__)
    #endif
#else
    #define LOG_WARNING(...) do {} while (0)
#endif

#ifdef PRINT_INFO
    #ifdef _LOG_HEADER
        #define LOG_INFO(...) _LOG_INFO_HEADER_STATEMENT(); \
        _GET_MACRO(__VA_ARGS__, _LOG_INFO1, _LOG_INFO2)(__VA_ARGS__)
    #else
        #define LOG_INFO(...) _GET_MACRO(__VA_ARGS__, _LOG_INFO1, _LOG_INFO2)(__VA_ARGS__)
    #endif
#else
    #define LOG_INFO(...) do {} while (0)
#endif

#ifdef PRINT_DEBUG
    #ifdef _LOG_HEADER
        #define LOG_DEBUG(...) _LOG_DEBUG_HEADER_STATEMENT(); \
                _GET_MACRO(__VA_ARGS__, _LOG_DEBUG1, _LOG_DEBUG2)(__VA_ARGS__)
    #else
        #define LOG_DEBUG(...) _GET_MACRO(__VA_ARGS__, _LOG_DEBUG1, _LOG_DEBUG2)(__VA_ARGS__)
    #endif
#else
    #define LOG_DEBUG(...) do {} while (0)
#endif

#ifdef PRINT_TRACE
    #ifdef _LOG_HEADER
        #define LOG_TRACE(...) _LOG_TRACE_HEADER_STATEMENT(); \
                _GET_MACRO(__VA_ARGS__, _LOG_TRACE1, _LOG_TRACE2)(__VA_ARGS__)
    #else
        #define LOG_TRACE(...) _GET_MACRO(__VA_ARGS__, _LOG_TRACE1, _LOG_TRACE2)(__VA_ARGS__)
    #endif
#else
    #define LOG_TRACE(...) do {} while (0)
#endif

#ifdef PRINT_FATAL
    #ifdef _LOG_HEADER
        #define LOG_FATAL(...) _LOG_FATAL_HEADER_STATEMENT(); \
        _GET_MACRO(__VA_ARGS__, _LOG_FATAL1, _LOG_FATAL2)(__VA_ARGS__)
    #else
        #define LOG_FATAL(...) _GET_MACRO(__VA_ARGS__, _LOG_FATAL1, _LOG_FATAL2)(__VA_ARGS__)
    #endif
#else
    #define LOG_FATAL(...) do {} while (0)
#endif 

#ifdef PRINT_VERBOSE
    #ifdef _LOG_HEADER
        #define LOG_VERBOSE(...) _LOG_VERBOSE_HEADER_STATEMENT(); \
        _GET_MACRO(__VA_ARGS__, _LOG_VERBOSE1, _LOG_VERBOSE2)(__VA_ARGS__)
    #else
        #define LOG_VERBOSE(...) _GET_MACRO(__VA_ARGS__, _LOG_VERBOSE1, _LOG_VERBOSE2)(__VA_ARGS__)
    #endif
#else
    #define LOG_VERBOSE(...) do {} while (0)
#endif



#endif 