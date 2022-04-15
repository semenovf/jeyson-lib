################################################################################
# Copyright (c) 2019-2022 Vladislav Trifochkin
#
# This file is part of `jeyson-lib`.
#
# Changelog:
#      2022.02.07 Initial version.
################################################################################
cmake_minimum_required (VERSION 3.11)
project(jeyson-lib C CXX)

option(JEYSON__ENABLE_JANSSON "Enable `Jansson` library for JSON support" ON)
option(JEYSON__EXCEPTIONS_DISABLED "Disable exceptions" OFF)

if (NOT TARGET pfs::common)
    portable_target(INCLUDE_PROJECT
        ${CMAKE_CURRENT_LIST_DIR}/3rdparty/pfs/common/library.cmake)
endif()

if (JEYSON__ENABLE_JANSSON AND NOT TARGET jansson)
    if (NOT JEYSON__JANSSON_ROOT)
        set(JEYSON__JANSSON_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/jansson" CACHE INTERNAL "")
    endif()

    portable_target(INCLUDE_PROJECT
        ${CMAKE_CURRENT_LIST_DIR}/cmake/Jansson.cmake)
endif()

portable_target(LIBRARY ${PROJECT_NAME} ALIAS pfs::jeyson)

portable_target(INCLUDE_DIRS ${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_LIST_DIR}/include)
portable_target(LINK ${PROJECT_NAME} PUBLIC pfs::common)
portable_target(EXPORTS ${PROJECT_NAME} JEYSON__EXPORTS JEYSON__STATIC)

if (JEYSON__EXCEPTIONS_DISABLED)
    portable_target(DEFINITIONS ${PROJECT_NAME} PUBLIC "JEYSON__EXCEPTIONS_DISABLED=1")
endif()

if (JEYSON__ENABLE_JANSSON)
    portable_target(SOURCES ${PROJECT_NAME}
        ${CMAKE_CURRENT_LIST_DIR}/src/jansson.cpp)
    portable_target(LINK ${PROJECT_NAME} PRIVATE jansson)
endif()

