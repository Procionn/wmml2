find_package(GTest REQUIRED)

add_executable(wmml_tests main.cpp constants.h)

if (WIN32)
    target_link_libraries(wmml_tests
        PRIVATE
        wmml
        GTest::gtest
        GTest::gtest_main
    )
elseif (UNIX)
    target_link_libraries(wmml_tests
        PRIVATE
        wmml
        GTest::gtest
        GTest::gtest_main
        pthread
    )
endif()

enable_testing()
add_test(NAME wmml_tests COMMAND wmml_tests)
