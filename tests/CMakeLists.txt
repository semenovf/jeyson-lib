################################################################################
# Copyright (c) 2019-2022 Vladislav Trifochkin
#
# This file is part of `jeyson-lib`.
#
# Changelog:
#      2019.12.11 Initial version.
#      2022.02.07 Refactored for using portable_target `ADD_TEST`.
#      2022.09.26 Added `iterator` test.
################################################################################
project(jeyson-TESTS)

# Copy test files to build folder
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/data DESTINATION .)

set(TESTS json iterator)

foreach (t ${TESTS})
    portable_target(ADD_TEST ${t}
        ENABLE_COVERAGE ${JEYSON__ENABLE_COVERAGE}
        SOURCES ${t}.cpp ${${t}_SOURCES}
        LINK pfs::jeyson)
endforeach()
