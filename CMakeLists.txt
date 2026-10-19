cmake_minimum_required(VERSION 3.16)
project(wmml LANGUAGES CXX)

option(BUILD_SHARED_LIBS "Build using shared libraries" ON)
set(CMAKE_INSTALL_PREFIX ${CMAKE_BINARY_DIR}/install CACHE PATH "Install path prefix" FORCE)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(src)
set(ENABLE_TESTING ON)
if(ENABLE_TESTING)
  add_subdirectory(tests)
endif()
