cmake_minimum_required(VERSION 3.16)
project(wmml LANGUAGES CXX)

set(CMAKE_INSTALL_PREFIX ${CMAKE_BINARY_DIR}/install CACHE PATH "Install path prefix" FORCE)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_compile_options(-O3)

target_include_directories(wmml PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

add_subdirectory(src)
if(ENABLE_TESTING)
  add_subdirectory(tests)
endif()

