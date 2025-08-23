################################################################################
# Copyright (c) 2025 Vladislav Trifochkin
#
# This file is part of `jeyson-lib`.
#
# Changelog:
#       2025.07.27 Initial version
################################################################################

if (JEYSON__ENABLE_JANSSON)
    if (JEYSON__DISABLE_FETCH_CONTENT AND EXISTS ${CMAKE_CURRENT_LIST_DIR}/jansson/.git)
        add_subdirectory(jansson 3rdparty/jansson)
    else()
        set(JANSSON_BUILD_SHARED_LIBS OFF CACHE BOOL "Build shared libraries")
        set(JANSSON_BUILD_DOCS    OFF CACHE BOOL "Disable build Jansson docs")
        set(JANSSON_EXAMPLES      OFF CACHE BOOL "Disable compile example applications")
        set(JANSSON_COVERAGE      OFF CACHE BOOL "Disable coverage support")
        set(JANSSON_WITHOUT_TESTS ON  CACHE BOOL "Disable tests")
        set(JANSSON_INSTALL       OFF CACHE BOOL "Disable installation")

        set(FETCHCONTENT_UPDATES_DISCONNECTED_JANSSON ON)
        message(STATUS "Fetching jansson ...")
        include(FetchContent)
        FetchContent_Declare(jansson
            GIT_REPOSITORY https://github.com/akheron/jansson.git
            GIT_TAG v2.14.1
            SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR}/jansson)
        FetchContent_MakeAvailable(jansson)
        message(STATUS "Fetching jansson complete")
    endif()
endif(JEYSON__ENABLE_JANSSON)
