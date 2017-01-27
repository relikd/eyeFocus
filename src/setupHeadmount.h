#ifndef SETUP_HEADMOUNT_H
#define SETUP_HEADMOUNT_H

#include "constants.h"
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

namespace Setup {
	class Headmount {
		std::vector<cv::Point> userPoints;
		const cv::String windowName;
		const char* savePath;
		
		
	public:
		Headmount(const cv::String window, const char* file = NULL) : windowName(window), savePath(file) {
			setMouseCallback(window, mouseHandler, &userPoints);
		}
		
		bool waitForInput(cv::Mat frame, RectPair *eyeRegion);
		
	private:
		static void mouseHandler(int event, int x, int y, int flags, void* param);
	};
}

#endif /* SETUP_HEADMOUNT_H */
