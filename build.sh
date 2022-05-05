#!/bin/sh

set -e

# build release
[ -e release ] || mkdir release

cd release
cmake -DCMAKE_BUILD_TYPE=Release ..
cmake --build .
cd -

# build debug

[ -e debug ] || mkdir debug

cd debug
cmake -DCMAKE_BUILD_TYPE=Debug ..
cmake --build .
cd -

# pack

cpack --config MultiCPackConfig.cmake