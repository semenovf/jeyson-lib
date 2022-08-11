################################################################################
# Copyright (c) 2019-2022 Vladislav Trifochkin
#
# This file is part of `jeyson-lib`.
#
# Changelog:
#      2022.02.07 Initial version.
################################################################################
cmake_minimum_required (VERSION 3.11)
project(jeyson C CXX)

option(JEYSON__ENABLE_JANSSON "Enable `Jansson` library for JSON support" ON)

if (NOT PORTABLE_TARGET__CURRENT_PROJECT_DIR)
    set(PORTABLE_TARGET__CURRENT_PROJECT_DIR ${CMAKE_CURRENT_SOURCE_DIR})
endif()

portable_target(ADD_SHARED ${PROJECT_NAME} ALIAS pfs::jeyson EXPORTS JEYSON__EXPORTS
    BIND_STATIC ${PROJECT_NAME}-static STATIC_ALIAS pfs::jeyson::static STATIC_EXPORTS JEYSON__STATIC)

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

    portable_target(INCLUDE_DIRS ${PROJECT_NAME} PRIVATE $<TARGET_PROPERTY:jansson,INCLUDE_DIRECTORIES>)
endif()

portable_target(INCLUDE_DIRS ${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_LIST_DIR}/include)
portable_target(LINK ${PROJECT_NAME} PUBLIC pfs::common)
portable_target(LINK ${PROJECT_NAME}-static PUBLIC pfs::common)

if (JEYSON__ENABLE_JANSSON)
    portable_target(SOURCES ${PROJECT_NAME}
        ${CMAKE_CURRENT_LIST_DIR}/src/jansson.cpp)
    portable_target(LINK ${PROJECT_NAME} PRIVATE jansson)
endif()

