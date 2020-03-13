project(pfs_json_TESTS)
option(pfs_json_COVERAGE "Build tests with coverage support" OFF)

include_directories(${CMAKE_SOURCE_DIR}/pfs-common/include)

if (pfs_json_COVERAGE)
    message(STATUS "Coverage feature enabled")
    if (NOT "${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
        message(FATAL_ERROR "Coverage feature requires GCC")
    endif()

    set(CMAKE_CXX_FLAGS "-g -O0 --coverage")
    set(CMAKE_C_FLAGS "-g -O0 --coverage")
endif()

# Copy test files to build folder
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/data DESTINATION .)

add_executable(json json.cpp)
add_test(NAME json COMMAND json)

add_executable(parser parser.cpp)
add_test(NAME parser COMMAND parser)

add_executable(file_parser file_parser.cpp)
add_test(NAME file_parser COMMAND file_parser)
