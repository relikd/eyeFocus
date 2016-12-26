#ifndef FIND_HEADMOUNT_H
#define FIND_HEADMOUNT_H

#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

typedef std::pair<cv::Rect2f, cv::Rect2f> TwoEyes;

namespace Detector {
	class Headmount {
	public:
		static TwoEyes askUserForInput(cv::VideoCapture cam, cv::String window);
	};
}

#endif /* FIND_HEADMOUNT_H */
