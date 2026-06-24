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
enable_testing()

# Simple test framework (self-contained, no external dependency)
add_library(test_framework STATIC tests/test_framework.cpp)
target_include_directories(test_framework PUBLIC ${CMAKE_SOURCE_DIR}/tests)

# Unit tests
function(add_unit_test name)
    add_executable(${name} tests/unit/${name}.cpp)
    target_link_libraries(${name} PRIVATE miniredis_lib test_framework)
    add_test(NAME ${name} COMMAND ${name})
endfunction()

add_unit_test(byte_buffer_test)
add_unit_test(result_test)
add_unit_test(resp_encoder_test)
add_unit_test(resp_parser_test)
add_unit_test(database_test)
add_unit_test(expiration_test)
add_unit_test(aof_test)
add_unit_test(config_test)

# Integration tests
function(add_integration_test name)
    add_executable(${name} tests/integration/${name}.cpp)
    target_link_libraries(${name} PRIVATE miniredis_lib test_framework)
    add_test(NAME ${name} COMMAND ${name})
endfunction()

add_integration_test(string_command_test)
add_integration_test(list_command_test)
add_integration_test(hash_command_test)
add_integration_test(ttl_command_test)
add_integration_test(session_test)
add_integration_test(pipeline_test)
add_integration_test(recovery_test)

# Blackbox TCP test (requires Python 3)
find_package(Python3 COMPONENTS Interpreter QUIET)
if(Python3_FOUND)
    add_test(NAME blackbox_tcp_test
        COMMAND ${Python3_EXECUTABLE} ${CMAKE_SOURCE_DIR}/tools/run_blackbox_tests.py
                --server $<TARGET_FILE:miniredis-server>
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
    set_tests_properties(blackbox_tcp_test PROPERTIES
        TIMEOUT 60
        LABELS "blackbox")
endif()
