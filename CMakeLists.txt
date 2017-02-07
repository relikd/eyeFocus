PROJECT( eyeFocus )
cmake_minimum_required(VERSION 3.1)

SET( EXECUTABLE_OUTPUT_PATH "../bin" CACHE PATH "Output directory" )

# Grouping
file( GLOB_RECURSE SRC_ROOT src/* *.(cpp|h|hpp) )
file( GLOB SRC_DETECTOR src/Detector/* *.(cpp|h|hpp) )
file( GLOB SRC_EXCUSE src/Detector/ExCuSe/* )
file( GLOB SRC_ELSE src/Detector/ElSe/* )
file( GLOB SRC_ESTIMATE src/Estimate/* )
file( GLOB SRC_HELPER src/Helper/* )
file( GLOB SRC_SETUP src/Setup/* )
source_group("" FILES ${SRC_ROOT})
source_group("Detector" FILES ${SRC_DETECTOR})
source_group("Detector\\ExCuSe" FILES ${SRC_EXCUSE})
source_group("Detector\\ElSe" FILES ${SRC_ELSE})
source_group("Estimate" FILES ${SRC_ESTIMATE})
source_group("Helper" FILES ${SRC_HELPER})
source_group("Setup" FILES ${SRC_SETUP})

if ("${CMAKE_CXX_COMPILER_ID}" MATCHES "MSVC")
	add_definitions( -D _USE_MATH_DEFINES ) # define M_PI etc for <cmath>
endif()

add_executable( eyeFocus ${SRC_ROOT} ) # ${SRC_DETECTOR} ${SRC_EXCUSE} ${SRC_ESTIMATE} ${SRC_HELPER} ${SRC_SETUP}
set_property( DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT eyeFocus )

# copy all files in res folder to bin at compile time
add_custom_command(
	TARGET eyeFocus
	POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy_directory
		${CMAKE_SOURCE_DIR}/res $<TARGET_FILE_DIR:eyeFocus>/res
)

if (WIN32)
	SET( CMAKE_PREFIX_PATH "$ENV{OPENCV_DIR}/lib" )
	set_target_properties( eyeFocus PROPERTIES LINK_FLAGS_DEBUG "/SUBSYSTEM:CONSOLE" )
	set_target_properties( eyeFocus PROPERTIES COMPILE_DEFINITIONS_DEBUG "_CONSOLE" )
endif()

if (APPLE)
	SET( CMAKE_PREFIX_PATH "/usr/local/Cellar/opencv3/3.2.0/share/OpenCV/" ) # OpenCVConfig.cmake
endif()

#set_property( TARGET eyeFocus PROPERTY CXX_STANDARD 11 )
target_compile_features( eyeFocus PRIVATE cxx_lambdas cxx_binary_literals cxx_nullptr cxx_range_for )

FIND_PACKAGE( OpenCV REQUIRED )
target_link_libraries( eyeFocus ${OpenCV_LIBS} )

# include_directories($ENV{OPENCV_DIR}/../../include)
# target_link_libraries( eyeFocus $ENV{OPENCV_DIR}/lib/opencv_world320d.lib )
