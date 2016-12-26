#ifndef findFace_h
#define findFace_h

#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/objdetect/objdetect.hpp>

typedef std::pair<cv::Rect2f, cv::Rect2f> TwoEyes;

namespace Detector {
	class Face {
		cv::CascadeClassifier face_cascade;
		
	public:
		Face(const cv::String name) {
			// Load the cascades
			if ( !face_cascade.load( name ) ) {
				printf("--(!)Error loading face cascade, please change face_cascade_name in source code.\n");
				exit(EXIT_FAILURE);
			}
		};
		cv::Rect findFace(cv::Mat &frame_gray);
		TwoEyes findEyes(cv::Mat &faceROI);
	};
}

#endif /* findFace_h */
