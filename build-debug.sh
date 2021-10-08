#!/bin/bash

set -e
set -x

rm -rf build
mkdir build
pushd build

conan install .. --build=missing
cmake .. -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Debug
cmake --build .
