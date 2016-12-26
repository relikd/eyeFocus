#ifndef findSkin_h
#define findSkin_h

#include <opencv2/imgproc/imgproc.hpp>

namespace Detector {
	class Skin {
	public:
		Skin();
		cv::Mat findSkin (cv::Mat &frame);
	};

}

#endif /* findSkin_h */
