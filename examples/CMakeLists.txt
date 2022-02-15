cmake_minimum_required(VERSION 3.8)

add_executable(jflect-example example.cpp)
target_link_libraries(jflect-example PRIVATE jflect)
target_compile_options(jflect-example PRIVATE "-Wall;-Werror;-Wextra;-Wpedantic")
