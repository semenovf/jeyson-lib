project(pfs_json_TESTS)
option(pfs_json_COVERAGE "Build tests with coverage support" OFF)

if (NOT TARGET pfs::json)
    message(FATAL_ERROR "No target `pfs::json` found")
endif()

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

set(TEST_NAMES
    json
    iterator
    parser
    file_parser)

foreach (name ${TEST_NAMES})
    if (${name}_SOURCES)
        add_executable(${name} ${name}.cpp ${${name}_SOURCES})
    else ()
        add_executable(${name} ${name}.cpp)
    endif()

    target_link_libraries(${name} PRIVATE pfs::json)
    add_test(NAME ${name} COMMAND ${name})
endforeach()
