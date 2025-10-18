set(SOURCE
    wmml.h
    base_wmml.h
    wmml.cpp
    wmml_marker.cpp
    wmml_archive_struct.cpp
    base_wmml.cpp
)

add_library(wmml_shared SHARED ${SOURCE})
target_compile_definitions(wmml_shared PRIVATE BUILDING_WMML_LIB)
target_include_directories(wmml_shared PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:include>
)


add_library(wmml_static STATIC ${SOURCE})
target_compile_definitions(wmml_static PRIVATE WMML_STATIC)
target_include_directories(wmml_static PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:include>
)
