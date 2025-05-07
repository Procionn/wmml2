cmake_minimum_required(VERSION 3.16)

project(wmml LANGUAGES CXX)

set(CMAKE_INSTALL_PREFIX ${CMAKE_BINARY_DIR}/install CACHE PATH "Install path prefix" FORCE)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(wmml SHARED       
	src/wmml.h    
	src/wmml.cpp
	src/wmml_marker.cpp
	src/wmml_archive_struct.cpp           
	src/base_wmml.h src/base_wmml.cpp
)

set_target_properties(wmml PROPERTIES PREFIX "")

target_include_directories(wmml PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:include>
)

target_compile_definitions(wmml PRIVATE BUILDING_WMML_LIB)

include(GNUInstallDirs)

install(TARGETS wmml
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(FILES wmml.h base_wmml.h
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)
