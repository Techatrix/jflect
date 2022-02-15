cmake_minimum_required(VERSION 3.11)

include(GoogleTest)

macro(package_add_test TESTNAME)
    add_executable(${TESTNAME} ${ARGN})
    
    target_link_libraries(${TESTNAME} PUBLIC gtest_main gmock)
    target_link_libraries(${TESTNAME} PUBLIC jflect)

    target_compile_options(${TESTNAME} PRIVATE "-Wall;-Werror;-Wextra;-Wpedantic")

    gtest_discover_tests(${TESTNAME})
endmacro()

package_add_test(jflect_test write.cpp read.cpp parser.cpp)
