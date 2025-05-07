#!/bin/bash
set -e

rm -rf cmake
mkdir cmake
cd cmake
cmake ..
make
cd ..