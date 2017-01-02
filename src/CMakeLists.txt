#find_library(MGL mgl PATHS /usr/local/lib)

ADD_EXECUTABLE( eyeLike main.cpp constants.h Debug.cpp Debug.h findEyeCenter.cpp findEyeCenter.h findEyeCorner.cpp findEyeCorner.h findHeadmount.cpp findHeadmount.h findFace.cpp findFace.h findSkin.cpp findSkin.h )
TARGET_LINK_LIBRARIES( eyeLike ${OpenCV_LIBS} )
#target_link_libraries( eyeLike ${MGL})

# C++11 support
target_compile_features(eyeLike PRIVATE cxx_range_for)