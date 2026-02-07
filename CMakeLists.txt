# Specify minimum cmake version
cmake_minimum_required(VERSION 3.5)

# project name and version
project(dewlog_tests VERSION 0.1.0 LANGUAGES C)
set(PROJECT_EXE_NAME ${PROJECT_NAME})

# specify C standard
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_EXTENSIONS False) # Without extensions False will set standard to gnu99 instead of c99.
set(CMAKE_C_STANDARD_REQUIRED True)

# Set built types
set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS Debug DebWithCov DebWithAsan)

if(CMAKE_GENERATOR MATCHES "Visual Studio|Xcode|Ninja Multi-Config")
    if(NOT CMAKE_CONFIGURATION_TYPES)
        set(CMAKE_CONFIGURATION_TYPES "Debug;DebWithCov;DebWithAsan" CACHE STRING "" FORCE)
    endif()
else()
    if(NOT CMAKE_BUILD_TYPE)
        set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build." FORCE)
    endif()
endif()

# FROM: https://github.com/libsdl-org/SDL/blob/main/docs/INTRO-cmake.md
# set the output directory for built objects.
# This makes sure that the dynamic library goes into the build directory automatically.
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")

# Enable shared libraries globally
set(BUILD_SHARED_LIBS ON)

# Check if cmocka exist, if not then run the dependency clone script
if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/external/cmocka")
    message(STATUS "Running clone script...")
    execute_process(
        COMMAND "${CMAKE_CURRENT_SOURCE_DIR}/scripts/clone_dependencies.sh" SDL_OFF
        WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
    )
endif()

# Set this to disable building cmocka examples (MOCKA_WITH_EXAMPLES does NOT work!)
set(WITH_EXAMPLES OFF CACHE BOOL "Disable CMocka examples")
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/external/cmocka)

# Add tests subdirectory (only on UNIX currently)
# if(BUILD_TESTS AND UNIX)
    add_subdirectory(tests)
# endif()
