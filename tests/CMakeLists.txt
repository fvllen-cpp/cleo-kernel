include(GoogleTest)

add_executable(cleo_kernel_tests 
    hello_test.cpp
)

get_target_property(GTEST_INCLUDE_DIRS GTest::gtest INTERFACE_INCLUDE_DIRECTORIES)

if(CMAKE_EXPORT_COMPILE_COMMANDS)
    list(APPEND CMAKE_CXX_STANDARD_INCLUDE_DIRECTORIES ${GTEST_INCLUDE_DIRS})
endif()

target_include_directories(cleo_kernel_tests SYSTEM PRIVATE
    ${GTEST_INCLUDE_DIRS}
)

target_link_libraries(cleo_kernel_tests PRIVATE
    cleo
    GTest::gtest_main
)

gtest_discover_tests(cleo_kernel_tests)
