#include <stddef.h>
#include <stdint.h>
#include <stdarg.h>
#include <setjmp.h>
#include <cmocka.h>

// #define DEWLOG_NO_FILE
// #define DEWLOG_NO_LINE
#define DEWLOG_IMPLEMENTATION
#include "dewlog.h"

extern const struct CMUnitTest dewlog_tests[];
extern const size_t dewlog_tests_count;

int main(void) {
    int fail = 0;

    /* Run the logger test group */
    fail += _cmocka_run_group_tests("dewlog.h tests", dewlog_tests, dewlog_tests_count, NULL, NULL);

    return fail;
}
