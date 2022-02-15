cmake_minimum_required(VERSION 3.11)

include(FetchContent)

#[[
FetchContent_Declare(
fmt
GIT_REPOSITORY https://github.com/fmtlib/fmt
GIT_TAG        master
)

FetchContent_MakeAvailable(fmt)
]]

FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest
    GIT_TAG        main
)

FetchContent_MakeAvailable(googletest)


FetchContent_Declare(
    googlebenchmark
    GIT_REPOSITORY https://github.com/google/benchmark
    GIT_TAG main
)

set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "Suppressing benchmark's tests" FORCE)        
FetchContent_MakeAvailable(googlebenchmark)


FetchContent_Declare(
        fastfloat
        GIT_REPOSITORY https://github.com/fastfloat/fast_float
        GIT_TAG        main
)

FetchContent_MakeAvailable(fastfloat)
