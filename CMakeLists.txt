cmake_minimum_required(VERSION 3.16)
project(MiniRedis17 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Compiler warnings
add_compile_options(-Wall -Wextra -Wpedantic -Wconversion -Wshadow)

# Sanitizer options
option(MINIREDIS_ENABLE_ASAN "Enable AddressSanitizer" OFF)
option(MINIREDIS_ENABLE_UBSAN "Enable UndefinedBehaviorSanitizer" OFF)

if(MINIREDIS_ENABLE_ASAN)
    add_compile_options(-fsanitize=address -fno-omit-frame-pointer)
    add_link_options(-fsanitize=address)
endif()

if(MINIREDIS_ENABLE_UBSAN)
    add_compile_options(-fsanitize=undefined)
    add_link_options(-fsanitize=undefined)
endif()

# Build type defaults
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Release")
    add_compile_options(-O2 -DNDEBUG)
else()
    add_compile_options(-g -O0)
endif()

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/include)

# Source files (library)
set(MINIREDIS_SOURCES
    src/resp/encoder.cpp
    src/resp/parser.cpp
    src/storage/database.cpp
    src/storage/expiration.cpp
    src/command/registry.cpp
    src/command/executor.cpp
    src/command/string_commands.cpp
    src/command/list_commands.cpp
    src/command/hash_commands.cpp
    src/command/expire_commands.cpp
    src/server/byte_buffer.cpp
    src/server/connection.cpp
    src/server/session.cpp
    src/server/event_loop.cpp
    src/server/server.cpp
    src/persistence/aof.cpp
    src/persistence/aof_replay.cpp
    src/config/config.cpp
)

# Static library
add_library(miniredis_lib STATIC ${MINIREDIS_SOURCES})
target_include_directories(miniredis_lib PUBLIC ${CMAKE_SOURCE_DIR}/include)

# Server executable
add_executable(miniredis-server src/main.cpp)
target_link_libraries(miniredis-server PRIVATE miniredis_lib)

# Testing
# Student starter repositories default to Task 0. Increase this with
# -DMINIREDIS_TASK_LEVEL=N as students advance through the course.
set(MINIREDIS_TASK_LEVEL 0 CACHE STRING "Highest MiniRedis course task whose public tests are enabled")
set_property(CACHE MINIREDIS_TASK_LEVEL PROPERTY STRINGS 0 1 2 3 4 5 6 7 8)

enable_testing()

# Simple test framework (self-contained, no external dependency)
add_library(test_framework STATIC tests/test_framework.cpp)
target_include_directories(test_framework PUBLIC ${CMAKE_SOURCE_DIR}/tests)

function(miniredis_task_enabled task out_var)
    if(${task} LESS_EQUAL ${MINIREDIS_TASK_LEVEL})
        set(${out_var} TRUE PARENT_SCOPE)
    else()
        set(${out_var} FALSE PARENT_SCOPE)
    endif()
endfunction()

# Unit tests
function(add_unit_test name task)
    miniredis_task_enabled(${task} enabled)
    if(enabled)
        add_executable(${name} tests/unit/${name}.cpp)
        target_link_libraries(${name} PRIVATE miniredis_lib test_framework)
        add_test(NAME ${name} COMMAND ${name})
        set_tests_properties(${name} PROPERTIES LABELS "task${task}")
    endif()
endfunction()

add_unit_test(byte_buffer_test 0)
add_unit_test(result_test 0)
add_unit_test(resp_encoder_test 1)
add_unit_test(resp_parser_test 2)
add_unit_test(database_string_test 3)
add_unit_test(database_test 4)
add_unit_test(config_test 6)
add_unit_test(expiration_test 7)
add_unit_test(aof_test 8)

# Integration tests
function(add_integration_test name task)
    miniredis_task_enabled(${task} enabled)
    if(enabled)
        add_executable(${name} tests/integration/${name}.cpp)
        target_link_libraries(${name} PRIVATE miniredis_lib test_framework)
        add_test(NAME ${name} COMMAND ${name})
        set_tests_properties(${name} PROPERTIES LABELS "task${task}")
    endif()
endfunction()

add_integration_test(string_command_test 3)
add_integration_test(list_command_test 4)
add_integration_test(hash_command_test 4)
add_integration_test(session_test 5)
add_integration_test(pipeline_test 5)
add_integration_test(ttl_command_test 7)
add_integration_test(recovery_test 8)

# Blackbox TCP tests (require Python 3). The core network test belongs to
# Task 6; the full test adds AOF restart recovery in Task 8.
find_package(Python3 COMPONENTS Interpreter QUIET)
if(Python3_FOUND)
    miniredis_task_enabled(6 blackbox_core_enabled)
    if(blackbox_core_enabled)
        add_test(NAME blackbox_tcp_core_test
            COMMAND ${Python3_EXECUTABLE} ${CMAKE_SOURCE_DIR}/tools/run_blackbox_tests.py
                    --server $<TARGET_FILE:miniredis-server> --skip-ttl --skip-aof
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
        set_tests_properties(blackbox_tcp_core_test PROPERTIES
            TIMEOUT 60
            LABELS "task6;blackbox")
    endif()

    miniredis_task_enabled(7 blackbox_ttl_enabled)
    if(blackbox_ttl_enabled)
        add_test(NAME blackbox_tcp_ttl_test
            COMMAND ${Python3_EXECUTABLE} ${CMAKE_SOURCE_DIR}/tools/run_blackbox_tests.py
                    --server $<TARGET_FILE:miniredis-server> --skip-aof
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
        set_tests_properties(blackbox_tcp_ttl_test PROPERTIES
            TIMEOUT 60
            LABELS "task7;blackbox")
    endif()

    miniredis_task_enabled(8 blackbox_full_enabled)
    if(blackbox_full_enabled)
        add_test(NAME blackbox_tcp_test
            COMMAND ${Python3_EXECUTABLE} ${CMAKE_SOURCE_DIR}/tools/run_blackbox_tests.py
                    --server $<TARGET_FILE:miniredis-server>
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
        set_tests_properties(blackbox_tcp_test PROPERTIES
            TIMEOUT 60
            LABELS "task8;blackbox")
    endif()
endif()
