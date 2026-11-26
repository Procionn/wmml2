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

enable_testing()

# add_executable(STREAM
#     stream.cpp
# )

# add_executable(WMML_TESTS
#     wmml2_tests.cpp
# )
# target_link_libraries(WMML_TESTS PUBLIC
#   WMML
# )


add_executable(WMML_GTEST
  wmml_gtest.cpp
)
target_compile_definitions(WMML_GTEST PRIVATE
  WMML_TEST_DATA_DIR="${CMAKE_SOURCE_DIR}/dataset"
)
target_link_libraries(WMML_GTEST PUBLIC
  WMML
  GTest::gtest
  GTest::gtest_main
)
add_test(NAME WMML_GTEST COMMAND WMML_GTEST)

