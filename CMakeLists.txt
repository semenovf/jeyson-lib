################################################################################
# Copyright (c) 2019-2025 Vladislav Trifochkin
#
# This file is part of `jeyson-lib`.
#
# Changelog:
#       2019.11.05 Initial version (pfs-json).
#       2022.02.07 Renamed into jeyson-lib.
#       2024.03.29 Added build 3rdparty subdirectory.
#       2024.11.23 Up to C++14 standard.
#                  Removed `portable_target` dependency.
#       2025.07.27 Added add_subdirectory for 3rdparty and 2ndparty dependencies.
################################################################################
cmake_minimum_required (VERSION 3.19)
project(jeyson-ALL CXX C)

option(JEYSON__BUILD_STRICT "Build with strict policies: C++ standard required, C++ extension is OFF etc" ON)
option(JEYSON__BUILD_TESTS "Build tests" OFF)
option(JEYSON__BUILD_SHARED "Enable build shared library" OFF)
option(JEYSON__ENABLE_JANSSON "Enable `Jansson` library for JSON support" ON)
option(JEYSON__DISABLE_FETCH_CONTENT "Disable fetch content if sources of dependencies already exists in the working tree (checks .git subdirectory)" ON)

if (JEYSON__BUILD_STRICT)
    if (NOT CMAKE_CXX_STANDARD)
        set(CMAKE_CXX_STANDARD 14)
    endif()

    set(CXX_STANDARD_REQUIRED ON)
    set(CMAKE_CXX_EXTENSIONS OFF) # use -std=c++XX rather than -std=gnu++XX
endif()

if (JEYSON__BUILD_TESTS)
    set(JEYSON__BUILD_SHARED ON)
endif()

get_property(_runtime_output_directory GLOBAL PROPERTY RUNTIME_OUTPUT_DIRECTORY)
get_property(_library_output_directory GLOBAL PROPERTY LIBRARY_OUTPUT_DIRECTORY)
get_property(_archive_output_directory GLOBAL PROPERTY ARCHIVE_OUTPUT_DIRECTORY)

if (NOT _runtime_output_directory)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/output)
endif()

if (NOT _library_output_directory)
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/output)
endif()

if (NOT _archive_output_directory)
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/output)
endif()

add_subdirectory(3rdparty)
add_subdirectory(2ndparty)
include(${CMAKE_CURRENT_LIST_DIR}/library.cmake)

if (JEYSON__BUILD_TESTS AND EXISTS ${CMAKE_CURRENT_LIST_DIR}/tests)
    enable_testing()
    add_subdirectory(tests)
endif()

include(GNUInstallDirs)

install(TARGETS jeyson
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
