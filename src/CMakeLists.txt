cmake_minimum_required(VERSION 3.16)

project(wmml2 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(wmml2 main.cpp
                            wmml.h wmml.cpp wmml_archive_struct.cpp wmml_marker.cpp
                            base_wmml.h base_wmml.cpp)

include(GNUInstallDirs)
install(TARGETS wmml2
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
