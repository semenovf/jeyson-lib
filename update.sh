#!/bin/bash

CWD=`pwd`
JANNSON_RELEASE=v2.14

if [ -e .git ] ; then

    git checkout master && git pull origin master \
        && git submodule update --init --recursive \
        && cd 3rdparty/portable-target && git checkout master && git pull origin master \
        && cd $CWD \
        && cd 3rdparty/pfs/common && git checkout master && git pull origin master \
        && cd $CWD \
        && cd 3rdparty/jansson && git checkout $JANNSON_RELEASE

fi
