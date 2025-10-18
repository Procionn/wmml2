find_package(GTest REQUIRED)

set(WMML_TESTS
    main.cpp
    constants.h
)
set(GTEST
    GTest::gtest
    GTest::gtest_main
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
