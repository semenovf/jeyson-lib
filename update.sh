#!/bin/bash

CWD=`pwd`
JANNSON_RELEASE=v2.14

if [ -e .git ] ; then

    git pull \
        && git submodule update --init \
        && git submodule update --remote \
        && cd 3rdparty/portable-target && git checkout master && git pull \
        && cd $CWD \
        && cd 3rdparty/pfs/common && git checkout master && git pull  \
        && cd $CWD \
        && cd 3rdparty/rocksdb && git checkout $JANNSON_RELEASE

fi

