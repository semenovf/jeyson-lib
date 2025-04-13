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
################################################################################
cmake_minimum_required (VERSION 3.19)
project(jeyson C CXX)

option(JEYSON__BUILD_SHARED "Enable build shared library" OFF)
option(JEYSON__ENABLE_JANSSON "Enable `Jansson` library for JSON support" ON)

if (JEYSON__BUILD_SHARED)
    add_library(jeyson SHARED)
    target_compile_definitions(jeyson PRIVATE JEYSON__EXPORTS)
else()
    add_library(jeyson STATIC)
    target_compile_definitions(jeyson PRIVATE JEYSON__STATIC)
endif()

add_library(pfs::jeyson ALIAS jeyson)

if (NOT TARGET pfs::common)
    set(FETCHCONTENT_UPDATES_DISCONNECTED_COMMON ON)

    include(FetchContent)
    FetchContent_Declare(common
        GIT_REPOSITORY https://github.com/semenovf/common-lib.git
        GIT_TAG master
        SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/2ndparty/common
        SUBBUILD_DIR ${CMAKE_CURRENT_BINARY_DIR}/2ndparty/common)
    FetchContent_MakeAvailable(common)
endif()

if (JEYSON__ENABLE_JANSSON)
    set(FETCHCONTENT_UPDATES_DISCONNECTED_JANSSON ON)

    set(JANSSON_BUILD_SHARED_LIBS OFF CACHE BOOL "Build shared libraries")
    set(JANSSON_BUILD_DOCS    OFF CACHE BOOL "Disable build Jansson docs")
    set(JANSSON_EXAMPLES      OFF CACHE BOOL "Disable compile example applications")
    set(JANSSON_COVERAGE      OFF CACHE BOOL "Disable coverage support")
    set(JANSSON_WITHOUT_TESTS ON  CACHE BOOL "Disable tests")
    set(JANSSON_INSTALL       OFF CACHE BOOL "Disable installation")

    include(FetchContent)
    FetchContent_Declare(jansson
        GIT_REPOSITORY https://github.com/akheron/jansson.git
        GIT_TAG v2.14
        SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/jansson
        SUBBUILD_DIR ${CMAKE_CURRENT_BINARY_DIR}/3rdparty/jansson)
    FetchContent_MakeAvailable(jansson)

    list(APPEND _jeyson__sources ${CMAKE_CURRENT_LIST_DIR}/src/jansson.cpp)
endif()

target_sources(jeyson PRIVATE ${_jeyson__sources})
target_include_directories(jeyson PUBLIC ${CMAKE_CURRENT_LIST_DIR}/include
    PRIVATE ${CMAKE_CURRENT_LIST_DIR}/include/pfs)
target_link_libraries(jeyson PUBLIC pfs::common)

if (JEYSON__ENABLE_JANSSON)
    target_link_libraries(jeyson PRIVATE jansson)
    target_include_directories(jeyson PRIVATE $<TARGET_PROPERTY:jansson,INCLUDE_DIRECTORIES>)
    target_compile_definitions(jeyson PUBLIC JEYSON__JANSSON_ENABLED=1)
endif()
