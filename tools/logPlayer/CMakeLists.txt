PROJECT( logPlayer )
cmake_minimum_required(VERSION 3.1)

source_group("" FILES main.cpp)
add_executable( logPlayer main.cpp )

if (WIN32)
	SET( CMAKE_PREFIX_PATH "$ENV{OPENCV_DIR}/lib" )
	set_target_properties( logPlayer PROPERTIES LINK_FLAGS_DEBUG "/SUBSYSTEM:CONSOLE" )
	set_target_properties( logPlayer PROPERTIES COMPILE_DEFINITIONS_DEBUG "_CONSOLE" )
endif()

if (APPLE)
	SET( CMAKE_PREFIX_PATH "/usr/local/Cellar/opencv3/3.2.0/share/OpenCV/" ) # OpenCVConfig.cmake
endif()

target_compile_features( logPlayer PRIVATE cxx_lambdas cxx_nullptr cxx_range_for )

FIND_PACKAGE( OpenCV REQUIRED )
target_link_libraries( logPlayer ${OpenCV_LIBS} )
