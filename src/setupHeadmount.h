#ifndef SETUP_HEADMOUNT_H
#define SETUP_HEADMOUNT_H

#include "constants.h"
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

namespace Setup {
	class Headmount {
	public:
		static RectPair askUserForInput(cv::VideoCapture cam, cv::String window);
	};
}

#endif /* SETUP_HEADMOUNT_H */
