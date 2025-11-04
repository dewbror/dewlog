/*
c_log.h - v0.1.0

Documentation:
    TODO: Write this...
*/

#ifndef C_LOG_H_
#define C_LOG_H_

#include <stdint.h> // Needed for fixed-width types

#ifdef __cplusplus
extern "C" {
#endif

#define C_LOG_LEVEL_ERROR 0
#define C_LOG_LEVEL_WARN  1
#define C_LOG_LEVEL_INFO  2
#define C_LOG_LEVEL_DEBUG 3
#define C_LOG_LEVEL_TRACE 4

/*
 * Open file 'file_name' for logging, 'file_name' == NULL will set logging to stderr.
 *
 * \param[in] file_name A string with the name of the log file to open.
 */
void c_log_open(const char *file_name);

/*
 * Close log file.
 */
void c_log_close(void);

// Check for GCC or Clang
#if defined(__GNUC__) || defined(__clang__)
#define FORMAT_ATTR(format_index, first_arg_index) __attribute__((format(printf, format_index, first_arg_index)))
#else
#define FORMAT_ATTR(format_index, first_arg_index) // NOP for MSVC and other compilers
#endif

/*
 * NOT MEANT TO BE USED! Use LOG_ERROR, _INFO, ... etc. instead.
 *
 * Valid C_LOG_LEVELS are:
 *     0: ERROR,
 *     1: WARNING,
 *     2: INFO,
 *     3: DEBUG,
 *     4 to UINT8_MAX: TRACE.
 *
 * \param[in] level The C_LOG_LEVEL of the message,
 * \param[in] fmt A printf-style message format string.
 * \param[in] ... Additional parameters matching % tokens in the "fmt" string, if any.
 */
void __c_log__msg(const uint8_t level, const char *const file, const int32_t line, const char *const func,
    const char *const fmt, ...) FORMAT_ATTR(5, 6);

#undef FORMAT_ATTR

// If C_LOG_LEVEL is not defined, use default C_LOG_LEVEL
#ifndef C_LOG_LEVEL
#ifdef NDEBUG
// Default C_LOG_LEVEL for NDEBUG mode is C_LOG_INFO
#define C_LOG_LEVEL C_LOG_LEVEL_INFO
#else
// Default C_LOG_LEVEL in debug mode is C_LOG_TRACE
#define C_LOG_LEVEL C_LOG_LEVEL_TRACE
#endif
#endif

#if C_LOG_LEVEL >= C_LOG_LEVEL_ERROR
#define LOG_ERROR(...) __c_log__msg(C_LOG_LEVEL_ERROR, __FILE__, __LINE__, __func__, __VA_ARGS__)
#else
#define LOG_ERROR(...) ((void)0)
#endif

#if C_LOG_LEVEL >= C_LOG_LEVEL_WARN
#define LOG_WARN(...) __c_log__msg(C_LOG_LEVEL_WARN, __FILE__, __LINE__, __func__, __VA_ARGS__)
#else
#define LOG_WARN(...) ((void)0)
#endif

#if C_LOG_LEVEL >= C_LOG_LEVEL_INFO
#define LOG_INFO(...) __c_log__msg(C_LOG_LEVEL_INFO, __FILE__, __LINE__, __func__, __VA_ARGS__)
#else
#define LOG_INFO(...) ((void)0)
#endif

#if C_LOG_LEVEL >= C_LOG_LEVEL_DEBUG
#define LOG_DEBUG(...) __c_log__msg(C_LOG_LEVEL_DEBUG, __FILE__, __LINE__, __func__, __VA_ARGS__)
#else
#define LOG_DEBUG(...) ((void)0)
#endif

#if C_LOG_LEVEL >= C_LOG_LEVEL_TRACE
#define LOG_TRACE(...) __c_log__msg(C_LOG_LEVEL_TRACE, __FILE__, __LINE__, __func__, __VA_ARGS__)
#else
#define LOG_TRACE(...) ((void)0)
#endif

#ifdef C_LOG_IMPLEMENTATION

#include <stdbool.h>
#include <stdio.h>
#include <stdarg.h>
#include <time.h>

#define TIMEBUF_MAX 32
#define INFOBUF_MAX 256

#define COLOR_RED    "\x1b[31m"
#define COLOR_YELLOW "\x1b[33m"
#define COLOR_GREEN  "\x1b[32m"
#define COLOR_BLUE   "\x1b[34m"
#define COLOR_CYAN   "\x1b[36m"
#define COLOR_RESET  "\x1b[0m"

// @TODO: Fix thread safety, global non-consts are not thread safe. Technically logging to stderr is thread safe but we
// should still make it excplicitly thread safe. My current plan is to make it thread safe with mutexes
static bool C_LOG_logging_to_file = false;
static FILE *C_LOG_fp = NULL;
static const char *C_LOG_file_name = NULL;

void c_log_open(const char *const p_file_name)
{
    // First check if we are already logging to file
    if(C_LOG_logging_to_file)
        return;

    // Null check file_name
    if(p_file_name == NULL) {
        C_LOG_logging_to_file = false;
        C_LOG_fp = stderr;
        return;
    }

    // If file_name != NULL, open it as logging file
    C_LOG_fp = fopen(p_file_name, "w");
    if(C_LOG_fp == NULL)
        return;

    // Set the name of the log file
    C_LOG_file_name = p_file_name;
    C_LOG_logging_to_file = true;

    LOG_INFO("Log file opened '%s'", C_LOG_file_name);
}

void c_log_close(void)
{
    if(!C_LOG_logging_to_file)
        return;

    LOG_DEBUG("Attempting to close log file '%s'", C_LOG_file_name);

    // Check if log_file is NULL
    if(C_LOG_fp == NULL)
        return;

    // Close log_file
    int ret = fclose(C_LOG_fp);
    if(ret != 0) {
        LOG_DEBUG("Failed to close log file '%s'", C_LOG_file_name);
        return;
    }

    // Set file ptr to default stderr
    C_LOG_fp = stderr;
    C_LOG_logging_to_file = false;
}

void __c_log__msg(const uint8_t level, const char *const file, const int32_t line, const char *const func,
    const char *const fmt, ...)
{
    // UNUSED
    (void)file;
    (void)line;

    // Null check fp_log
    if(C_LOG_fp == NULL)
        return;

    int ret = 0;
    size_t ret_LU = 0;
    const char *color = NULL;

    // Set the color based on the log level of the message
    switch(level) {
    case C_LOG_LEVEL_ERROR:
        color = COLOR_RED;
        break;
    case C_LOG_LEVEL_WARN:
        color = COLOR_YELLOW;
        break;
    case C_LOG_LEVEL_INFO:
        color = COLOR_GREEN;
        break;
    case C_LOG_LEVEL_DEBUG:
        color = COLOR_BLUE;
        break;
    case C_LOG_LEVEL_TRACE:
    default:
        color = COLOR_CYAN;
        break;
    }

    // Get the current time
    time_t now = time(NULL);
    struct tm *tm_now = localtime(&now);

    // Format time string
    char timebuf[TIMEBUF_MAX];
    ret_LU = strftime(timebuf, sizeof(timebuf), "%Y-%m-%d %H:%M:%S", tm_now);
    if(ret_LU == 0)
        return;

    char infobuf[INFOBUF_MAX];
    // ret = snprintf(infobuf, sizeof(infobuf), "%s:%d [%s]", file, line, func);
    ret = snprintf(infobuf, sizeof(infobuf), "[%s]", func);

    // TODO: check if ret < INFOBUF_MAX?
    if(ret == 0)
        return;

    // 

    // Print date, time, log-level and info
#define C_LOG_LEVEL_COUNT 5
    static const char *const levels[C_LOG_LEVEL_COUNT] = {"[ERR] ", "[WRN] ", "[INF] ", "[DBG] ", "[TRC] "};
    if(level >= C_LOG_LEVEL_COUNT)
        level = C_LOG_LEVEL_COUNT - 1;
    if(C_LOG_logging_to_file) {
        ret = fprintf(C_LOG_fp, "%s %s%s", timebuf, levels[level], infobuf);
        if(ret < 0)
            return;
    } else {
        ret = fprintf(C_LOG_fp, "%s %s%s%s%s ", timebuf, color, levels[level], COLOR_RESET,
            infobuf);
        if(ret < 0)
            return;
    }

    // Null check format string
    if(fmt == NULL)
        goto NEWLINE;

    // Initialize variable argument list
    va_list args;
    va_start(args, fmt);

    // Print formatted string
    ret = vfprintf(C_LOG_fp, fmt, args);
    if(ret < 0) {
        va_end(args);
        goto NEWLINE;
    }

    // Cleanup va lists
    va_end(args);

NEWLINE:

    // Print newline
    ret = fprintf(C_LOG_fp, "\n");
    if(ret < 0)
        return;
}

// Undef macros
#undef TIMEBUF_MAX
#undef INFOBUF_MAX

#undef COLOR_RED
#undef COLOR_YELLOW
#undef COLOR_GREEN
#undef COLOR_CYAN
#undef COLOR_RESET

#undef C_LOG_LEVEL_COUNT

#endif // C_LOG_IMPLEMENTATION

// #undef C_LOG_LEVEL_ERROR
// #undef C_LOG_LEVEL_WARN
// #undef C_LOG_LEVEL_INFO
// #undef C_LOG_LEVEL_DEBUG
// #undef C_LOG_LEVEL_TRACE

#ifdef __cplusplus
}
#endif

#endif // C_LOG_H_

/*
Version History:
    0.1.0 (2025-10-01) First released version.
*/

/*
Copyright (c) 2025 dewbror <dewbror@gmail.com>

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the “Software”), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
of the Software, and to permit persons to whom the Software is furnished to do
so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/
