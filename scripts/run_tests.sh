#!/bin/sh

tests_exe="c_log_tests"
file="c_log"

dir=""

if [ -f "./build_debwithcov/DebWithCov/${tests_exe}" ]; then
    tests="./build_debwithcov/DebWithCov/${tests_exe}"
    dir="./build_debwithcov"
elif [ -f "./build_debwithasan/DebWithAsan/${tests_exe}" ]; then
    tests="./build_debwithasan/DebWithAsan/${tests_exe}"
    dir="./build_debug"
elif [ -f "./build_debug/Debug/${tests_exe}" ]; then
    tests="./build_debug/Debug/${tests_exe}"
    dir="./build_debug"
elif [ -f "./build_release/Release/${tests_exe}" ]; then
    tests="./build_release/Release/${tests_exe}"
    dir="./build_release"
else
    echo "Test build not found"
    exit 1
fi

echo "Running ${tests}"
"${tests}" || exit 1
echo ""

if [ "${dir}" = "./build_debwithcov" ]; then
    echo "Generating coverage info"
    lcov --capture --directory "${dir}/tests/CMakeFiles" --output-file "${dir}/coverage.info" > /dev/null 2>&1 || exit 1
    lcov --extract "${dir}/coverage.info" "*/${file}.h" --output-file "${dir}/coverage.${file}.info" 2>/dev/null || exit 1
    genhtml "${dir}/coverage.${file}.info" --output-directory "${dir}/coverage_html" > /dev/null 2>/dev/null || exit 1
    rm "${dir}/coverage.info" || exit 1
fi
