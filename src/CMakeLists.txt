set(SOURCE
    wmml.h
    base_wmml.h
    wmml.cpp
    wmml_marker.cpp
    wmml_archive_struct.cpp
    base_wmml.cpp
)

set(FLAGS
    -O3
    -flto
)

add_library(wmml_shared SHARED ${SOURCE})
target_compile_definitions(wmml_shared PUBLIC WMML_SHARED)
target_compile_definitions(wmml_shared PRIVATE BUILDING_WMML_LIB)
target_compile_options(wmml_shared PRIVATE ${FLAGS})
target_include_directories(wmml_shared PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:include>
)

add_library(wmml_static STATIC ${SOURCE})
target_compile_options(wmml_static PRIVATE ${FLAGS} -ffat-lto-objects)
target_include_directories(wmml_static PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:include>
)
