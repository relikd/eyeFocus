#include "findEyeCorner.h"
//#include <opencv2/highgui/highgui.hpp> // debug window

using namespace Detector;

cv::Point2f EyeCorner::find(cv::Mat region, bool left, bool left2) {
	cv::Mat cornerMap = eyeCornerMap(region, left, left2);
	
	cv::Point maxP;
	cv::minMaxLoc(cornerMap,NULL,NULL,NULL,&maxP);
	
	cv::Point2f maxP2;
	maxP2 = findSubpixel(cornerMap, maxP);
	// GFTT
//	std::vector<cv::Point2f> corners;
//	cv::goodFeaturesToTrack(region, corners, 500, 0.005, 20);
//	for (int i = 0; i < corners.size(); ++i) {
//		cv::circle(region, corners[i], 2, 200);
//	}
//	imshow("Corners",region);
	
	return maxP2;
}

// TODO: implement these
cv::Mat EyeCorner::eyeCornerMap(const cv::Mat &region, bool left, bool left2) {
	cv::Mat cornerMap;
	
	cv::Size sizeRegion = region.size();
	cv::Range colRange(sizeRegion.width / 4, sizeRegion.width * 3 / 4);
	cv::Range rowRange(sizeRegion.height / 4, sizeRegion.height * 3 / 4);
	
	cv::Mat miRegion(region, rowRange, colRange);
	
	cv::filter2D(miRegion, cornerMap, CV_32F,
				 (left && !left2) || (!left && !left2) ? *leftCornerKernel : *rightCornerKernel);
	
	return cornerMap;
}

cv::Point2f EyeCorner::findSubpixel(cv::Mat region, cv::Point maxP) {
	
	cv::Size sizeRegion = region.size();
	
	// Matrix dichotomy
	// Not useful, matrix becomes too small
	
	/*int offsetX = 0;
	if (maxP.x - sizeRegion.width / 4 <= 0) {
		offsetX = 0;
	} else if (maxP.x + sizeRegion.width / 4 >= sizeRegion.width) {
		offsetX = sizeRegion.width / 2 - 1;
	} else {
		offsetX = maxP.x - sizeRegion.width / 4;
	}
	int offsetY = 0;
	if (maxP.y - sizeRegion.height / 4 <= 0) {
		offsetY = 0;
	} else if (maxP.y + sizeRegion.height / 4 >= sizeRegion.height) {
		offsetY = sizeRegion.height / 2 - 1;
	} else {
		offsetY = maxP.y - sizeRegion.height / 4;
	}
	cv::Range colRange(offsetX, offsetX + sizeRegion.width / 2);
	cv::Range rowRange(offsetY, offsetY + sizeRegion.height / 2);
	
	cv::Mat miRegion(region, rowRange, colRange);
	
	
	if (left) {
		imshow("aa", miRegion); // create new window for display
	} else {
		imshow("aaa", miRegion);
	}*/
	
	cv::Mat cornerMap(sizeRegion.height * 10, sizeRegion.width * 10, CV_32F);
	
	cv::resize(region, cornerMap, cornerMap.size(), 0, 0, cv::INTER_CUBIC);
	
	cv::Point maxP2;
	cv::minMaxLoc(cornerMap, NULL,NULL,NULL,&maxP2);
	
	return cv::Point2f(sizeRegion.width / 2 + maxP2.x / 10,
					   sizeRegion.height / 2 + maxP2.y / 10);
}
