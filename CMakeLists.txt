################################################################################
# Copyright (c) 2019-2024 Vladislav Trifochkin
#
# This file is part of `jeyson-lib`.
#
# Changelog:
#       2019.11.05 Initial version (pfs-json).
#       2022.02.07 Renamed into jeyson-lib.
#       2024.03.29 Added build 3rdparty subdirectory.
#       2024.11.23 Up to C++14 standard.
#                  Removed `portable_target` dependency.
################################################################################
cmake_minimum_required (VERSION 3.19)
project(jeyson-ALL CXX C)

option(JEYSON__BUILD_STRICT "Build with strict policies: C++ standard required, C++ extension is OFF etc" ON)
option(JEYSON__BUILD_TESTS "Build tests" OFF)

if (JEYSON__BUILD_STRICT)
    if (NOT CMAKE_CXX_STANDARD)
        set(CMAKE_CXX_STANDARD 14)
    endif()

    set(CXX_STANDARD_REQUIRED ON)
    set(CMAKE_CXX_EXTENSIONS OFF) # use -std=c++XX rather than -std=gnu++XX
endif()

message(STATUS "C++ compiler: ${CMAKE_CXX_COMPILER}")
message(STATUS "C++ compiler version: ${CMAKE_CXX_COMPILER_VERSION}")

if (JEYSON__BUILD_TESTS)
    set(JEYSON__BUILD_SHARED ON)
endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/output)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_LIBRARY_OUTPUT_DIRECTORY})

include(${CMAKE_CURRENT_LIST_DIR}/library.cmake)

if (JEYSON__BUILD_TESTS)
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
