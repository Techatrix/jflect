add_executable(benchmark_exe benchmark.cpp)
target_link_libraries(benchmark_exe jflect benchmark::benchmark)
target_compile_options(benchmark_exe PRIVATE "-Wall;-Werror;-Wextra;-Wpedantic")

