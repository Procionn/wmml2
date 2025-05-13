#!/bin/bash
set -e

rm -rf cmake
mkdir cmake
cd cmake
cmake ..
make

cd tests
ctest

cd ../..
rm -rf out
mkdir out
cp cmake/src/libwmml.so out/libwmml.so
cp src/wmml.h out/wmml.h
cp src/base_wmml.h out/base_wmml.h
