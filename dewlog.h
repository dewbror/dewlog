/*
dewlog.h - v0.3

Documentation:
    TODO: Write this...
*/

#ifndef DEWLOG_H_
#define DEWLOG_H_

#ifdef __cplusplus
extern "C" {
#endif

#define DEWLOG_LEVEL_ERROR 0
#define DEWLOG_LEVEL_WARN  1
#define DEWLOG_LEVEL_INFO  2
#define DEWLOG_LEVEL_DEBUG 3
#define DEWLOG_LEVEL_TRACE 4

/*
 * Open file 'file_name' for logging, 'file_name' == NULL will set logging to stderr.
 *
 * \param[in] file_name A string with the name of the log file to open.
 */
void dewlog_open(const char *file_name);

/*
 * Close log file.
 */
void dewlog_close(void);

/* Check for GCC or Clang */
#if defined(__GNUC__) || defined(__clang__)
#define FORMAT_ATTR(format_index, first_arg_index) __attribute__((format(printf, format_index, first_arg_index)))
#else
#define FORMAT_ATTR(format_index, first_arg_index) /* NOP for MSVC and other compilers */
#endif

/*
 * NOT MEANT TO BE USED! Use LOG_ERROR, _INFO, ... etc. instead.
 *
 * Valid DEWLOG_LEVELS are:
 *     0: ERROR,
 *     1: WARNING,
 *     2: INFO,
 *     3: DEBUG,
 *     4: TRACE.
 *
 * \param[in] level The DEWLOG_LEVEL of the message,
 * \param[in] fmt A printf-style message format string.
 * \param[in] ... Additional parameters matching % tokens in the "fmt" string, if any.
 */
void __dewlog__msg(const int level, const char *const file, const int line, const char *const func,
    const char *const fmt, ...) FORMAT_ATTR(5, 6);

#undef FORMAT_ATTR

/* If DEWLOG_LEVEL is not defined, use default DEWLOG_LEVEL */
#ifndef DEWLOG_LEVEL
#ifdef NDEBUG
/* Default DEWLOG_LEVEL for NDEBUG mode is DEWLOG_LEVEL_INFO */
#define DEWLOG_LEVEL DEWLOG_LEVEL_INFO
#else
/* Default DEWLOG_LEVEL in debug mode is DEWLOG_LEVEL_TRACE */
#define DEWLOG_LEVEL DEWLOG_LEVEL_TRACE
#endif
#endif

#if DEWLOG_LEVEL >= DEWLOG_LEVEL_ERROR
#define LOG_ERROR(...) __dewlog__msg(DEWLOG_LEVEL_ERROR, __FILE__, __LINE__, __func__, __VA_ARGS__)
#else
#define LOG_ERROR(...) ((void)0)
#endif

#if DEWLOG_LEVEL >= DEWLOG_LEVEL_WARN
#define LOG_WARN(...) __dewlog__msg(DEWLOG_LEVEL_WARN, __FILE__, __LINE__, __func__, __VA_ARGS__)
#else
#define LOG_WARN(...) ((void)0)
#endif

#if DEWLOG_LEVEL >= DEWLOG_LEVEL_INFO
#define LOG_INFO(...) __dewlog__msg(DEWLOG_LEVEL_INFO, __FILE__, __LINE__, __func__, __VA_ARGS__)
#else
#define LOG_INFO(...) ((void)0)
#endif

#if DEWLOG_LEVEL >= DEWLOG_LEVEL_DEBUG
#define LOG_DEBUG(...) __dewlog__msg(DEWLOG_LEVEL_DEBUG, __FILE__, __LINE__, __func__, __VA_ARGS__)
#else
#define LOG_DEBUG(...) ((void)0)
#endif

#if DEWLOG_LEVEL >= DEWLOG_LEVEL_TRACE
#define LOG_TRACE(...) __dewlog__msg(DEWLOG_LEVEL_TRACE, __FILE__, __LINE__, __func__, __VA_ARGS__)
#else
#define LOG_TRACE(...) ((void)0)
#endif

#ifdef DEWLOG_IMPLEMENTATION

#include <stdio.h>
#include <stdarg.h>
#include <time.h>
#include <string.h>

#define LOGBUF_MAX 512

#define COLOR_RED    "\x1b[31m"
#define COLOR_YELLOW "\x1b[33m"
#define COLOR_GREEN  "\x1b[32m"
#define COLOR_BLUE   "\x1b[34m"
#define COLOR_CYAN   "\x1b[36m"
#define COLOR_RESET  "\x1b[0m"

/* TODO: Add thread safe access to static data (via #ifdef DEWLOG_THREAD_SAFE_*) */
static int DEWLOG_logging_to_file = 0;
static FILE *DEWLOG_fp = NULL;
static const char *DEWLOG_file_name = NULL;

void dewlog_open(const char *const p_file_name)
{
    /* First check if we are already logging to file */
    if(DEWLOG_logging_to_file)
        return;

    /* Null check file_name */
    if(p_file_name == NULL) {
        DEWLOG_logging_to_file = 0;
        DEWLOG_fp = stderr;
        return;
    }

    /* If file_name != NULL, open it as logging file */
    DEWLOG_fp = fopen(p_file_name, "w");
    if(DEWLOG_fp == NULL)
        return;

    /* Set the name of the log file */
    DEWLOG_file_name = p_file_name;
    DEWLOG_logging_to_file = 1;

    LOG_INFO("Log file opened '%s'", DEWLOG_file_name);
}

void dewlog_close(void)
{
    if(!DEWLOG_logging_to_file)
        return;

    LOG_DEBUG("Attempting to close log file '%s'", DEWLOG_file_name);

    /* Check if log_file is NULL */
    if(DEWLOG_fp == NULL)
        return;

    /* Close log_file */
    int ret = fclose(DEWLOG_fp);
    if(ret != 0) {
        LOG_DEBUG("Failed to close log file '%s'", DEWLOG_file_name);
        return;
    }

    /* Set file ptr to default stderr */
    DEWLOG_fp = stderr;
    DEWLOG_logging_to_file = 0;
}

void __dewlog__msg(const int level, const char *const file, const int line, const char *const func,
    const char *const fmt, ...)
{
    // Unused hack
    (void)file;
    (void)line;
    (void)func;

    /* Null check fp_log */
    if(DEWLOG_fp == NULL)
        return;

    const char *color = NULL;

    /* Set the color based on the log level of the message */
    switch(level) {
    case DEWLOG_LEVEL_ERROR:
        color = COLOR_RED;
        break;
    case DEWLOG_LEVEL_WARN:
        color = COLOR_YELLOW;
        break;
    case DEWLOG_LEVEL_INFO:
        color = COLOR_GREEN;
        break;
    case DEWLOG_LEVEL_DEBUG:
        color = COLOR_BLUE;
        break;
    case DEWLOG_LEVEL_TRACE:
    default:
        color = COLOR_CYAN;
        break;
    }

    int ret = 0;
    size_t sret = 0;

    time_t now = time(NULL);
    struct tm *tm_now = localtime(&now);
    char logbuf[LOGBUF_MAX];

    sret = strftime(logbuf, sizeof(logbuf), "%Y-%m-%d %H:%M:%S ", tm_now);
    if(sret == 0)
        goto FMT_STR;

#ifndef DEWLOG_FILE_MAX_LEN
#define DEWLOG_FILE_MAX_LEN 35
#endif

#ifndef DEWLOG_FUNC_MAX_LEN
#define DEWLOG_FUNC_MAX_LEN 25
#endif

#ifndef DEWLOG_NO_FILE
    size_t file_len = strlen(file);
    if(file_len > DEWLOG_FILE_MAX_LEN) {
        ret = snprintf(logbuf + sret, sizeof(logbuf) - sret, "...%*s", DEWLOG_FILE_MAX_LEN - 3,
            file + file_len - (DEWLOG_FILE_MAX_LEN - 3));
    }
    else {
        ret = snprintf(logbuf + sret, sizeof(logbuf) - sret, "%*s", DEWLOG_FILE_MAX_LEN, file);
    }
    if(ret <= 0)
        goto FMT_STR;
    sret += (size_t)ret;
#endif
#ifndef DEWLOG_NO_LINE
    ret = snprintf(logbuf + sret, sizeof(logbuf) - sret, ":%4d ", line);
    if(ret <= 0)
        goto FMT_STR;
    sret += (size_t)ret;
#endif
#ifndef DEWLOG_NO_FUNC
    size_t func_len = strlen(func);
    if(func_len > DEWLOG_FUNC_MAX_LEN) {
        ret = snprintf(logbuf + sret, sizeof(logbuf) - sret, "[...%*s]", DEWLOG_FUNC_MAX_LEN - 3,
            func + func_len - (DEWLOG_FUNC_MAX_LEN - 3));
    }
    else {
        ret = snprintf(logbuf + sret, sizeof(logbuf) - sret, "[%*s]", DEWLOG_FUNC_MAX_LEN, func);
    }
    if(ret <= 0)
        goto FMT_STR;
    sret += (size_t)ret;
#endif

#undef DEWLOG_FILE_MAX_LEN
#undef DEWLOG_FUNC_MAX_LEN

#define DEWLOG_LEVEL_COUNT 5
    static const char *levels[DEWLOG_LEVEL_COUNT] = {"[ERR]", "[WRN]", "[INF]", "[DBG]", "[TRC]"};
    if(DEWLOG_logging_to_file) {
        if(level < DEWLOG_LEVEL_COUNT)
            ret = fprintf(DEWLOG_fp, "%s %s ", logbuf, levels[level]);
        else
            ret = fprintf(DEWLOG_fp, "%s %s ", logbuf, levels[DEWLOG_LEVEL_COUNT - 1]);
    }
    else {
        if(level < DEWLOG_LEVEL_COUNT)
            ret = fprintf(DEWLOG_fp, "%s %s%s%s ", logbuf, color, levels[level], COLOR_RESET);
        else
            ret = fprintf(DEWLOG_fp, "%s %s%s%s ", logbuf, color, levels[DEWLOG_LEVEL_COUNT - 1], COLOR_RESET);
    }

FMT_STR:

    if(!fmt)
        goto NEWLINE;

    va_list args;
    va_start(args, fmt);

    ret = vfprintf(DEWLOG_fp, fmt, args);
    if(ret < 0) {
        va_end(args);
        goto NEWLINE;
    }

    va_end(args);

NEWLINE:

    ret = fprintf(DEWLOG_fp, "\n");
    if(ret < 0)
        return;
}

/* Undef macros */
#undef LOGBUF_MAX

#undef COLOR_RED
#undef COLOR_YELLOW
#undef COLOR_GREEN
#undef COLOR_CYAN
#undef COLOR_RESET

#undef DEWLOG_LEVEL_COUNT

#endif /* DEWLOG_IMPLEMENTATION */

/* #undef DEWLOG_LEVEL_ERROR */
/* #undef DEWLOG_LEVEL_WARN  */
/* #undef DEWLOG_LEVEL_INFO  */
/* #undef DEWLOG_LEVEL_DEBUG */
/* #undef DEWLOG_LEVEL_TRACE */

#ifdef __cplusplus
}
#endif

#endif /* DEWLOG_H_ */

/*
Version History:
    0.3 (2026-02-08) Added macros DEWLOG_NO_FILE, DEWLOG_NO_LINE and DEWLOG_NO_FUNC to disable printing of file name,
                     line number and function name.
                     DEWLOG_FILE_MAX_LEN and DEWLOG_FUNC_MAX_LEN macros added to limit the length of file and
                     function name. Default values are
                        - DEWLOG_FILE_MAX_LEN 35
                        - DEWLOG_FUNC_MAX_LEN 25
    0.2 (2026-02-07) Changed name c_log -> dewlog.
    0.1 (2025-10-01) First released version.
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
