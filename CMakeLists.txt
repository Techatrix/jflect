cmake_minimum_required(VERSION 3.11)

project(jflect
        VERSION 0.1.0
        DESCRIPTION "Json through static reflection"
        LANGUAGES CXX)

option(JFLECT_BUILD_EXAMPLES "Build Examples" ON)
option(JFLECT_BUILD_TESTS "Build Tests" ON)
option(JFLECT_BUILD_BENCHMARKS "Build Benchmarks" ON)

add_subdirectory(thirdparty)

add_library(jflect INTERFACE)
target_include_directories(jflect INTERFACE include)
target_link_libraries(jflect INTERFACE fast_float)
target_compile_features(jflect INTERFACE cxx_std_20)

if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME OR MODERN_CMAKE_BUILD_TESTING)
	if(JFLECT_BUILD_EXAMPLES)
		add_subdirectory(examples)
	endif()
	if(JFLECT_BUILD_TESTS)
	    enable_testing()
		add_subdirectory(tests)
	endif()
	if(JFLECT_BUILD_BENCHMARKS)
  		add_subdirectory(benchmark)
	endif()
endif()