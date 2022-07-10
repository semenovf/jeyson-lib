#!/bin/bash

CWD=`pwd`
JANNSON_RELEASE=v2.14

if [ -e .git ] ; then

    git checkout master && git pull origin master \
        && git submodule update --init \
        && git submodule update --remote \
        && cd 3rdparty/portable-target && git checkout master && git pull \
        && cd $CWD \
        && cd 3rdparty/pfs/common && ./update.sh  \
        && cd $CWD \
        && cd 3rdparty/jansson && git checkout $JANNSON_RELEASE

fi

