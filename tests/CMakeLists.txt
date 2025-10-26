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


set(WMML_TESTS
    main.cpp
    constants.h
)
set(GTEST
    GTest::gtest
    GTest::gtest_main
    GTest::gmock
    GTest::gmock_main
)
add_executable(wmml_tests_shared ${WMML_TESTS})
add_executable(wmml_tests_static ${WMML_TESTS})

if (WIN32)
    target_link_libraries(wmml_tests_shared PRIVATE
        wmml_shared
        ${GTEST}
    )
    target_link_libraries(wmml_tests_static PRIVATE
        wmml_static
        ${GTEST}
    )
elseif (UNIX)
    target_link_libraries(wmml_tests_shared PRIVATE
        wmml_shared
        ${GTEST}
        pthread
    )
    target_link_libraries(wmml_tests_static PRIVATE
        wmml_static
        ${GTEST}
        pthread
    )
endif()

enable_testing()
add_test(NAME wmml_tests_shared COMMAND wmml_tests_shared)
add_test(NAME wmml_tests_static COMMAND wmml_tests_static)
