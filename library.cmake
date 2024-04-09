################################################################################
# Copyright (c) 2019-2024 Vladislav Trifochkin
#
# This file is part of `jeyson-lib`.
#
# Changelog:
#      2022.02.07 Initial version.
#      2023.02.10 Separated static and shared builds.
#      2024.03.29 Replaced the sequence of two target configurations with a foreach statement.
################################################################################
cmake_minimum_required (VERSION 3.11)
project(jeyson C CXX)

option(JEYSON__BUILD_SHARED "Enable build shared library" OFF)
option(JEYSON__BUILD_STATIC "Enable build static library" ON)
option(JEYSON__ENABLE_JANSSON "Enable `Jansson` library for JSON support" ON)

if (NOT PORTABLE_TARGET__CURRENT_PROJECT_DIR)
    set(PORTABLE_TARGET__CURRENT_PROJECT_DIR ${CMAKE_CURRENT_SOURCE_DIR})
endif()

if (JEYSON__BUILD_SHARED)
    portable_target(ADD_SHARED ${PROJECT_NAME} ALIAS pfs::jeyson EXPORTS JEYSON__EXPORTS)
    list(APPEND _jeyson__targets ${PROJECT_NAME})
endif()

if (JEYSON__BUILD_STATIC)
    set(STATIC_PROJECT_NAME ${PROJECT_NAME}-static)
    portable_target(ADD_STATIC ${STATIC_PROJECT_NAME} ALIAS pfs::jeyson::static EXPORTS JEYSON__STATIC)
    list(APPEND _jeyson__targets ${STATIC_PROJECT_NAME})
endif()

if (NOT TARGET pfs::common)
    portable_target(INCLUDE_PROJECT ${CMAKE_CURRENT_LIST_DIR}/2ndparty/common/library.cmake)
endif()

if (JEYSON__ENABLE_JANSSON)
    if (NOT TARGET jansson)
        portable_target(INCLUDE_PROJECT ${CMAKE_CURRENT_LIST_DIR}/3rdparty/jansson.cmake)
    endif()

    list(APPEND _jeyson__sources ${CMAKE_CURRENT_LIST_DIR}/src/jansson.cpp)
endif()

foreach(_target IN LISTS _jeyson__targets)
    portable_target(SOURCES ${_target} ${_jeyson__sources})
    portable_target(INCLUDE_DIRS ${_target} PUBLIC ${CMAKE_CURRENT_LIST_DIR}/include)
    portable_target(LINK ${_target} PUBLIC pfs::common)

    if (JEYSON__ENABLE_JANSSON)
        portable_target(LINK ${_target} PRIVATE jansson)
        portable_target(INCLUDE_DIRS ${_target} PRIVATE $<TARGET_PROPERTY:jansson,INCLUDE_DIRECTORIES>)
        portable_target(DEFINITIONS ${_target} PUBLIC JEYSON__JANSSON_ENABLED=1)
    endif()
endforeach()
