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
#       2026.03.29 Merged with library.cmake.
################################################################################
cmake_minimum_required (VERSION 3.19)
project(jeyson CXX C)

option(JEYSON__BUILD_STRICT "Build with strict policies: C++ standard required, C++ extension is OFF etc" ON)
option(JEYSON__BUILD_STATIC "Force build static library" OFF)
option(JEYSON__BUILD_TESTS "Build tests" OFF)
option(JEYSON__ENABLE_JANSSON "Enable `Jansson` library for JSON support" ON)
option(JEYSON__DISABLE_FETCH_CONTENT "Disable fetch content if sources of dependencies already exists in the working tree (checks .git subdirectory)" ON)

if (JEYSON__BUILD_STRICT)
    if (NOT CMAKE_CXX_STANDARD)
        set(CMAKE_CXX_STANDARD 14)
    endif()

    set(CXX_STANDARD_REQUIRED ON)
    set(CMAKE_CXX_EXTENSIONS OFF) # use -std=c++XX rather than -std=gnu++XX
endif()

add_subdirectory(3rdparty)
add_subdirectory(2ndparty)

if (BUILD_SHARED_LIBS AND NOT JEYSON__BUILD_STATIC)
    add_library(jeyson)
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
