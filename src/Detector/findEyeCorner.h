#ifndef EYE_CORNER_H
#define EYE_CORNER_H

#include <opencv2/imgproc/imgproc.hpp>
#include "../Helper/KalmanPoint.h"

namespace Detector {
	class EyeCorner {
		// not constant because stupid opencv type signatures
		float kEyeCornerKernel[4][6] = {
			{-1,-1,-1, 1, 1, 1},
			{-1,-1,-1,-1, 1, 1},
			{-1,-1,-1,-1, 0, 3},
			{ 1, 1, 1, 1, 1, 1},
		};
		cv::Mat cornerKernel;
		KalmanPoint KF = KalmanPoint(1e-5, 30, 1000);
		
	public:
		EyeCorner(bool isRightEye = false) : cornerKernel(cv::Mat(4, 6, CV_32F, kEyeCornerKernel)) {
			if (isRightEye) flipKernelToRightCorner();
		};
		
		void flipKernelToRightCorner() {
			cv::flip(cornerKernel, cornerKernel, 1); // flip horizontally
		}
		
		cv::Point2f findByAvgColor(const cv::Mat &region, cv::Point2i offset);
		
	private:
		/** @return Corner position with subpixel accuracy. */
		cv::Point2f find(const cv::Mat &region);
		/** Try multiple 'highest value' corners. */
		cv::Point likeliestCorner(const cv::Mat &filtered);
	};
}

#endif
