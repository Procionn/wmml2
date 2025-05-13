cmake_minimum_required(VERSION 3.16)
project(wmml LANGUAGES CXX)

set(CMAKE_INSTALL_PREFIX ${CMAKE_BINARY_DIR}/install CACHE PATH "Install path prefix" FORCE)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(src)
add_subdirectory(tests)

# add_library(wmml SHARED
# 	wmml.h
# 	base_wmml.h
# 	wmml.cpp
# 	wmml_marker.cpp
# 	wmml_archive_struct.cpp
# 	base_wmml.cpp
# )
#
# target_include_directories(wmml PUBLIC
#     $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
#     $<INSTALL_INTERFACE:include>
# )
#
# target_compile_definitions(wmml PRIVATE BUILDING_WMML_LIB)
#
# include(GNUInstallDirs)
#
# install(TARGETS wmml
#     LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
#     RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
#     INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
# )
#
# install(FILES wmml.h base_wmml.h
#     DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
# )
