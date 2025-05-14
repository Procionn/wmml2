git clone https://github.com/google/googletest.git
cd googletest
mkdir build
cd build
cmake -G Ninja -DCMAKE_INSTALL_PREFIX="C:/libs/gtest" ..
ninja
ninja install
$env:CMAKE_PREFIX_PATH = "C:/libs/gtest"
cd ../..