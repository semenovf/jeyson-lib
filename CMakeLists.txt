cmake_minimum_required (VERSION 3.5.1)
project(pfs-json CXX)

option(pfs_json_BUILD_TESTS "Build tests" ON)
option(pfs_json_COVERAGE "Build tests with coverage support" OFF)

if (pfs_json_BUILD_TESTS)
    enable_testing()
    include_directories(${CMAKE_CURRENT_LIST_DIR}/include)
    add_subdirectory(tests)
endif()
