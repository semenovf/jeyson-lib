option(pfs_json_COVERAGE "Build tests with coverage support" OFF)

if (pfs_json_COVERAGE)
    message(STATUS "Coverage feature enabled")
    if (NOT "${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
        message(FATAL_ERROR "Coverage feature requires GCC")
    endif()

    set(CMAKE_CXX_FLAGS "--coverage -g -O0")
endif()

add_executable(json json.cpp)
add_test(NAME json COMMAND json)

add_executable(parser parser.cpp)
add_test(NAME parser COMMAND parser)
