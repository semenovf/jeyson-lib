################################################################################
# Copyright (c) 2022 Vladislav Trifochkin
#
# This file is part of `jeyson-lib`.
#
# Changelog:
#      2022.02.04 Initial version.
################################################################################
cmake_minimum_required (VERSION 3.11)

set(JANSSON_BUILD_SHARED_LIBS OFF CACHE BOOL "Build shared libraries")
set(JANSSON_BUILD_DOCS    OFF CACHE BOOL "Disable build Jansson docs")
set(JANSSON_EXAMPLES      OFF CACHE BOOL "Disable compile example applications")
set(JANSSON_COVERAGE      OFF CACHE BOOL "Disable coverage support")
set(JANSSON_WITHOUT_TESTS ON  CACHE BOOL "Disable tests")
set(JANSSON_INSTALL       OFF CACHE BOOL "Disable installation")

if (NOT JEYSON__JANSSON_ROOT)
    set(JEYSON__JANSSON_ROOT ${CMAKE_CURRENT_LIST_DIR}/3rdparty/jansson)
endif()

add_subdirectory(${JEYSON__JANSSON_ROOT} jansson)

target_include_directories(jansson PUBLIC $<TARGET_FILE_DIR:jansson>/../include)

if (CMAKE_COMPILER_IS_GNUCXX)
    # For link custom shared libraries with Jansson static library
    target_compile_options(jansson PRIVATE "-fPIC")
endif()
