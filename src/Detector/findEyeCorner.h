#ifndef EYE_CORNER_H
#define EYE_CORNER_H

#include <opencv2/imgproc/imgproc.hpp>
#include "../Helper/KalmanPoint.hpp"

namespace Detector {
	class EyeCorner {
		// not constant because stupid opencv type signatures
		float kEyeCornerKernel[4][6] = {
			{-1,-1,-1, 1, 1, 1},
			{-1,-1,-1,-1, 1, 1},
			{-1,-1,-1,-1, 0, 3},
			{ 1, 1, 1, 1, 1, 1},
		};
		
		cv::Mat *leftCornerKernel;
		cv::Mat *rightCornerKernel;
		
		KalmanPoint kflc = KalmanPoint(1e-5, 30, 1000); // left corner
		KalmanPoint kfrc = KalmanPoint(1e-5, 30, 1000); // right corner
		
	public:
		EyeCorner() {
			rightCornerKernel = new cv::Mat(4, 6, CV_32F, kEyeCornerKernel);
			leftCornerKernel = new cv::Mat(4, 6, CV_32F);
			// flip horizontally
			cv::flip(*rightCornerKernel, *leftCornerKernel, 1);
		};
		
		~EyeCorner() {
			delete leftCornerKernel;
			delete rightCornerKernel;
		};
		
		
		cv::Point2f find(cv::Mat region, bool left, bool left2);
		cv::Point2f findByAvgColor(cv::Mat region, bool left);
		
	private:
		cv::Mat eyeCornerMap(const cv::Mat &region, bool left, bool left2);
		cv::Point2f findSubpixel(cv::Mat region, cv::Point maxP);
	};
}

#endif
