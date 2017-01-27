PROJECT( eyeFocus )
cmake_minimum_required(VERSION 3.1)

SET( EXECUTABLE_OUTPUT_PATH "../bin" CACHE PATH "Output directory" )

file( GLOB_RECURSE SOURCE_FILES src/* )
# file( GLOB RES_FILES res/* )

if ("${CMAKE_CXX_COMPILER_ID}" MATCHES "MSVC")
	add_definitions( -D _USE_MATH_DEFINES ) # define M_PI etc for <cmath>
endif()

add_executable( eyeFocus ${SOURCE_FILES} ) # ${RES_FILES}
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
