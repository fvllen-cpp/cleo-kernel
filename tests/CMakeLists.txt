include(GoogleTest)

add_executable(cleo_kernel_tests 
    hello_test.cpp
)

target_link_libraries(cleo_kernel_tests PRIVATE
    cleo
    GTest::gtest_main
)

gtest_discover_tests(cleo_kernel_tests)
