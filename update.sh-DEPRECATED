#!/bin/bash

CWD=`pwd`
JANNSON_RELEASE=v2.14

if [ -e .git ] ; then

    git checkout master && git pull origin master \
        && git submodule update --init --recursive \
        && git submodule update --init --recursive --remote -- 3rdparty/portable-target \
        && git submodule update --init --recursive --remote -- 3rdparty/pfs/common \
        && cd $CWD \
        && cd 3rdparty/jansson && git checkout $JANNSON_RELEASE

fi
