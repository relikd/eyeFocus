#ifndef EYE_CENTER_H
#define EYE_CENTER_H

#include <opencv2/imgproc/imgproc.hpp>

namespace Detector {
	class EyeCenter {
	public:
		static cv::Point2f findEyeCenter(cv::Mat face, cv::Rect eye, std::string debugWindow);
	};
}

#endif
