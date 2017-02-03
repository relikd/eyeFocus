#ifndef setupSingleEye_hpp
#define setupSingleEye_hpp

#include "constants.h"
#include <opencv2/imgproc/imgproc.hpp>

namespace Setup {
	class SingleEye {
		std::vector<cv::Point2f> pupilAverage;
		const cv::String windowName;
		int internalSetIndex = 0;
		
	public:
		cv::Point2f cm20 = cv::Point2f(0,0);
		cv::Point2f cm50 = cv::Point2f(0,0);
		cv::Point2f cm80 = cv::Point2f(0,0);
		
		SingleEye(const cv::String window) : windowName(window) {};
		
		bool waitForInput(cv::Mat frame, cv::Point2f pupil);
	};
}

#endif /* setupSingleEye_hpp */
