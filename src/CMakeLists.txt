set(SOURCE
    wmml2.cpp
    stream.cpp
)


if (BUILD_SHARED_LIBS)
    set(FLAGS
        -O3
        # -flto
    )
else ()
    set(FLAGS
        -O3
        -flto
        -ffat-lto-objects
    )
endif()


add_library(WMML ${SOURCE})
target_compile_options(WMML PRIVATE ${FLAGS})
target_include_directories(WMML PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:include>
)
