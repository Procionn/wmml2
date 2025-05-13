add_library(wmml SHARED
	wmml.h
	base_wmml.h
	wmml.cpp
	wmml_marker.cpp
	wmml_archive_struct.cpp
	base_wmml.cpp
)

target_include_directories(wmml PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:include>
)
