################################################################################
# Copyright (c) 2019-2023 Vladislav Trifochkin
#
# This file is part of `jeyson-lib`.
#
# Changelog:
#      2022.02.07 Initial version.
#      2023.02.10 Separated static and shared builds.
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
endif()

if (JEYSON__BUILD_STATIC)
    set(STATIC_PROJECT_NAME ${PROJECT_NAME}-static)
    portable_target(ADD_STATIC ${STATIC_PROJECT_NAME} ALIAS pfs::jeyson::static EXPORTS JEYSON__STATIC)
endif()

if (NOT TARGET pfs::common)
    portable_target(INCLUDE_PROJECT
        ${PORTABLE_TARGET__CURRENT_PROJECT_DIR}/3rdparty/pfs/common/library.cmake)
endif()

if (JEYSON__ENABLE_JANSSON AND NOT TARGET jansson)
    if (NOT JEYSON__JANSSON_ROOT)
        set(JEYSON__JANSSON_ROOT "${PORTABLE_TARGET__CURRENT_PROJECT_DIR}/3rdparty/jansson" CACHE INTERNAL "")
    endif()

    portable_target(INCLUDE_PROJECT
        ${PORTABLE_TARGET__CURRENT_PROJECT_DIR}/cmake/Jansson.cmake)

    if (JEYSON__BUILD_SHARED)
        portable_target(INCLUDE_DIRS ${PROJECT_NAME} PRIVATE $<TARGET_PROPERTY:jansson,INCLUDE_DIRECTORIES>)
    endif()

    if (JEYSON__BUILD_STATIC)
        portable_target(INCLUDE_DIRS ${STATIC_PROJECT_NAME} PRIVATE $<TARGET_PROPERTY:jansson,INCLUDE_DIRECTORIES>)
    endif()

    list(APPEND _jeyson__sources ${CMAKE_CURRENT_LIST_DIR}/src/jansson.cpp)
endif()

if (JEYSON__BUILD_SHARED)
    portable_target(SOURCES ${PROJECT_NAME} ${_jeyson__sources})
    portable_target(INCLUDE_DIRS ${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_LIST_DIR}/include)
    portable_target(LINK ${PROJECT_NAME} PUBLIC pfs::common)

    if (JEYSON__ENABLE_JANSSON)
        portable_target(LINK ${PROJECT_NAME} PRIVATE jansson)
    endif()
endif()

if (JEYSON__BUILD_STATIC)
    portable_target(SOURCES ${STATIC_PROJECT_NAME} ${_jeyson__sources})
    portable_target(INCLUDE_DIRS ${STATIC_PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_LIST_DIR}/include)
    portable_target(LINK ${STATIC_PROJECT_NAME} PUBLIC pfs::common)

    if (JEYSON__ENABLE_JANSSON)
        portable_target(LINK ${STATIC_PROJECT_NAME} PRIVATE jansson)
    endif()
endif()
