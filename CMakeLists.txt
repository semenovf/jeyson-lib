################################################################################
# Copyright (c) 2019-2024 Vladislav Trifochkin
#
# This file is part of `jeyson-lib`.
#
# Changelog:
#      2019.11.05 Initial version (pfs-json).
#      2022.02.07 Renamed into jeyson-lib.
#      2024.03.29 Added build 3rdparty subdirectory.
################################################################################
cmake_minimum_required (VERSION 3.11)
project(jeyson-ALL CXX C)

option(JEYSON__BUILD_STRICT "Build with strict policies: C++ standard required, C++ extension is OFF etc" ON)
option(JEYSON__BUILD_TESTS "Build tests" OFF)
option(JEYSON__BUILD_DEMO "Build examples/demo" OFF)
option(JEYSON__ENABLE_COVERAGE "Build tests with coverage support" OFF)


if (NOT PORTABLE_TARGET__ENABLED)
    list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/2ndparty/portable-target/cmake/v2)
    include(PortableTarget)
    portable_target(SET ENABLE_TRACE ON)
endif()

message(STATUS "C++ compiler: ${CMAKE_CXX_COMPILER}")
message(STATUS "C++ compiler version: ${CMAKE_CXX_COMPILER_VERSION}")

if (JEYSON__BUILD_STRICT)
    set(CXX_STANDARD_REQUIRED ON)
    set(CMAKE_CXX_EXTENSIONS OFF) # use -std=c++XX rather than -std=gnu++XX
endif()

portable_target(INCLUDE_PROJECT ${CMAKE_CURRENT_LIST_DIR}/library.cmake)

if (JEYSON__BUILD_TESTS AND EXISTS ${CMAKE_CURRENT_LIST_DIR}/tests)
    enable_testing()
    add_subdirectory(tests)
endif()

if (JEYSON__BUILD_DEMO AND EXISTS ${CMAKE_CURRENT_LIST_DIR}/demo)
    add_subdirectory(demo)
endif()
