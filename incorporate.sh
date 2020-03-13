#!/bin/sh

HEADERS="iterator.hpp \
    variant.hpp \
    3rdparty/variant.hpp"

SOURCE_ROOT_DIR="../pfs-common/include/pfs"
TARGET_ROOT_DIR=`pwd`/include/pfs

. ../pfs-common/incorporate.inc
