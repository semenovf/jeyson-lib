#!/bin/sh

if [[ "${COMPILER}" != "" ]]; then
    export CXX=${COMPILER};
fi

uname -a
$CXX --version

mkdir -p build && cd build
cmake -GNinja -DCMAKE_CXX_STANDARD=${CXX_STANDARD} -DCMAKE_BUILD_TYPE=Release .
cmake --build . && ctest --output-on-failure
cd ..
