PROJECT( logEvaluator )
cmake_minimum_required(VERSION 3.1)

if ("${CMAKE_CXX_COMPILER_ID}" MATCHES "MSVC")
	add_definitions( -D _USE_MATH_DEFINES ) # define M_PI etc for <cmath>
endif()

source_group("" FILES main.cpp)
add_executable( logEvaluator main.cpp )
target_compile_features( logEvaluator PRIVATE cxx_lambdas cxx_nullptr cxx_range_for )
