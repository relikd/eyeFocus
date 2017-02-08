#include "findEyeCorner.h"

using namespace Detector;

cv::Point2f EyeCorner::findByAvgColor(cv::Mat region, bool left, cv::Point2f offset) {
	if (region.cols == 0 && region.rows == 0)
		return cv::Point2f(0,0);
	
	double avgValue = cv::sum(region)[0] / (region.cols * region.rows);
	cv::Mat tmp;
	//cv::Canny(tmp, tmp, 0.1, 0.1);
	cv::GaussianBlur(region, tmp, cv::Size(9,9), 0, 0);
	cv::threshold(tmp, tmp, avgValue * 0.95, 255, cv::THRESH_BINARY);
	cv::GaussianBlur(tmp, tmp, cv::Size(9,9), 0, 0);
	
	return (left ? kflc : kfrc).smoothedPosition( find(tmp, left, left) ) + offset;
}

cv::Point2f EyeCorner::find(cv::Mat region, bool left, bool left2) {
	cv::Mat cornerMap = eyeCornerMap(region, left, left2);
	
	cv::Point maxP;
	cv::minMaxLoc(cornerMap,NULL,NULL,NULL,&maxP);
	
	// GFTT
//	std::vector<cv::Point2f> corners;
//	cv::goodFeaturesToTrack(region, corners, 500, 0.005, 20);
//	for (int i = 0; i < corners.size(); ++i) {
//		cv::circle(region, corners[i], 2, 200);
//	}
//	imshow("Corners",region);
	
	return findSubpixel(cornerMap, maxP);
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
	
	cv::Mat cornerMap(sizeRegion.height * 10, sizeRegion.width * 10, CV_32F);
	
	cv::resize(region, cornerMap, cornerMap.size(), 0, 0, cv::INTER_CUBIC);
	
	cv::Point maxP2;
	cv::minMaxLoc(cornerMap, NULL,NULL,NULL,&maxP2);
	
	return cv::Point2f(sizeRegion.width / 2.0F + maxP2.x / 10.0F,
					   sizeRegion.height / 2.0F + maxP2.y / 10.0F);
}
