set(SOURCE
    wmml2.cpp
    stream.cpp
)

set(FLAGS
    -O3
    -flto
    -ffat-lto-objects
)


add_library(WMML ${SOURCE})
target_compile_options(WMML PRIVATE ${FLAGS})
target_include_directories(WMML PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:include>
)
