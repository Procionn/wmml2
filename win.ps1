rm -r cmake
mkdir cmake
cd cmake
cmake -G Ninja ..
ninja

cp src/libwmml.dll tests/libwmml.dll
cd tests
ctest

cd ../..
rm -r out
mkdir out
cp cmake/src/libwmml.dll out/libwmml.dll
cp src/wmml.h out/wmml.h
cp src/base_wmml.h out/base_wmml.h