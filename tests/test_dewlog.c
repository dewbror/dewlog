#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <setjmp.h>
#include <cmocka.h>

#define DEWLOG_LEVEL 4
#include "dewlog.h"

#define BUFFER_SIZE 512
#define TEMP_LOG_FILE "./temp.log"

// Helper to read the log file into a buffer
static void read_log_file(char *buffer, size_t bufsize)
{
    FILE *f = fopen(TEMP_LOG_FILE, "r");
    assert_non_null(f);
    size_t n = fread(buffer, 1, bufsize - 1, f);
    buffer[n] = 0;
    fclose(f);
}

static int setup(void **state)
{
    // UNUSED
    (void)state;

    dewlog_open(TEMP_LOG_FILE);

    return 0;
}

static int teardown(void **state)
{
    // UNUSED
    (void)state;

    dewlog_close();
    remove(TEMP_LOG_FILE);

    return 0;
}

// Test that the log file has been created (in setup)
static void test_log_file_creation(void **state)
{
    // UNUSED
    (void)state;

    FILE *f = fopen(TEMP_LOG_FILE, "r");
    assert_non_null(f);

    fclose(f);

    dewlog_close();

    char buf[BUFFER_SIZE];
    read_log_file(buf, sizeof(buf));
    assert_true(strstr(buf, "[INF]") != NULL);
    assert_true(strstr(buf, "Log file opened") != NULL);
}

static void test_log_message_written(void **state)
{
    // UNUSED
    (void)state;

    LOG_ERROR("Test message: %d", 42);
    dewlog_close();

    char buf[BUFFER_SIZE];
    read_log_file(buf, sizeof(buf));
    assert_true(strstr(buf, "Test message: 42") != NULL);
    assert_true(strstr(buf, "[ERR] ") != NULL);
}

static void test_log_level_prefix(void **state)
{
    // UNUSED
    (void)state;

    LOG_ERROR("Err!");
    LOG_WARN("Warn!");
    LOG_INFO("Info!");
    LOG_DEBUG("Debug!");
    LOG_TRACE("Trace!");
    dewlog_close();

    char buf[BUFFER_SIZE];
    read_log_file(buf, sizeof(buf));
    assert_true(strstr(buf, "[ERR] ") != NULL);
    assert_true(strstr(buf, "[WRN] ") != NULL);
    assert_true(strstr(buf, "[INF] ") != NULL);
    assert_true(strstr(buf, "[DBG] ") != NULL);
    assert_true(strstr(buf, "[TRC] ") != NULL);
}

const struct CMUnitTest dewlog_tests[] = {
    cmocka_unit_test_setup_teardown(test_log_file_creation, setup, teardown),
    cmocka_unit_test_setup_teardown(test_log_message_written, setup, teardown),
    cmocka_unit_test_setup_teardown(test_log_level_prefix, setup, teardown),
};

const size_t dewlog_tests_count = sizeof(dewlog_tests) / sizeof(dewlog_tests[0]);
