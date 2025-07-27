################################################################################
# Copyright (c) 2019-2024 Vladislav Trifochkin
#
# This file is part of `jeyson-lib`.
#
# Changelog:
#       2022.02.07 Initial version.
#       2023.02.10 Separated static and shared builds.
#       2024.03.29 Replaced the sequence of two target configurations with a foreach statement.
#       2024.11.23 Removed `portable_target` dependency.
#       2025.07.27 Moved building dependencies to 2ndparty and 3rdparty directories.
################################################################################
cmake_minimum_required (VERSION 3.19)
project(jeyson C CXX)

if (JEYSON__BUILD_SHARED)
    add_library(jeyson SHARED)
    target_compile_definitions(jeyson PRIVATE JEYSON__EXPORTS)
else()
    add_library(jeyson STATIC)
    target_compile_definitions(jeyson PRIVATE JEYSON__STATIC)
endif()

add_library(pfs::jeyson ALIAS jeyson)

target_include_directories(jeyson PUBLIC ${CMAKE_CURRENT_LIST_DIR}/include
    PRIVATE ${CMAKE_CURRENT_LIST_DIR}/include/pfs)
    
target_link_libraries(jeyson PUBLIC pfs::common)

if (JEYSON__ENABLE_JANSSON)
    target_sources(jeyson PRIVATE ${CMAKE_CURRENT_LIST_DIR}/src/jansson.cpp)
    target_link_libraries(jeyson PRIVATE jansson)
    target_include_directories(jeyson PRIVATE $<TARGET_PROPERTY:jansson,INCLUDE_DIRECTORIES>)
    target_compile_definitions(jeyson PUBLIC JEYSON__JANSSON_ENABLED=1)
endif()
