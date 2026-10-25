include(FetchContent)
FetchContent_Declare(
  GTest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG b2b9072ecbe874f5937054653ef8f2731eb0f010
)
FetchContent_MakeAvailable(GTest)

include_directories(
  ${GTEST_SOURCE_DIR}/googletest/include
  ${GTEST_SOURCE_DIR}/googlemock/include
)



# enable_testing()


add_executable(STREAM
    stream.cpp
)

add_executable(WMML_TESTS
    wmml2_tests.cpp
)

# target_compile_definitions(wmml_static PUBLIC # it will be used to test stream's stack overflowing
#   -DSTREAM_SIZE=32
# )

target_link_libraries(STREAM PUBLIC
  WMML
)

target_link_libraries(WMML_TESTS PUBLIC
  WMML
)
