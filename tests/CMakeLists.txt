################################################################################
# Copyright (c) 2019-2024 Vladislav Trifochkin
#
# This file is part of `jeyson-lib`.
#
# Changelog:
#       2019.12.11 Initial version.
#       2022.02.07 Refactored for using portable_target `ADD_TEST`.
#       2022.09.26 Added `iterator` test.
#       2024.11.23 Removed `portable_target` dependency.
################################################################################
project(jeyson-TESTS CXX)

# Copy test files to build folder
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/data DESTINATION ${CMAKE_BINARY_DIR}/output)

set(TESTS json iterator)

foreach (name ${TESTS})
    add_executable(${name} ${${name}_SOURCES} ${name}.cpp)
    target_link_libraries(${name} PRIVATE pfs::jeyson)
endforeach()
