//
//  findEyeCorner.cpp
//  eyeFocus
//
//  Created by Oleg Geier on 08/02/17.
//
//

#include "findEyeCorner.h"

using namespace Detector;

cv::Point2f EyeCorner::findByAvgColor(const cv::Mat &region, cv::Point2i offset) {
	if (region.cols == 0 || region.rows == 0)
		return cv::Point2f(0,0);
	
	float avgValue = cv::mean(region)[0];
	cv::Rect2i subSize = cv::Rect2i(region.cols/4, region.rows/4, region.cols/2, region.rows/2);
	
	cv::Mat tmp;
	cv::GaussianBlur(region(subSize), tmp, cv::Size(9,9), 0, 0);
	cv::threshold(tmp, tmp, avgValue * 0.95, 255, cv::THRESH_BINARY);
	cv::GaussianBlur(tmp, tmp, cv::Size(9,9), 0, 0); // blur a second time to get a smoother edge
	//cv::Canny(tmp, tmp, 0.1, 0.1);
	
	cv::Point2f corner = find(tmp);
	corner.x += subSize.x + offset.x;
	corner.y += subSize.y + offset.y;
	return KF.smoothedPosition( corner );
}

cv::Point2f EyeCorner::find(const cv::Mat &region) {
	// Apply filter kernel
	cv::Mat filteredImage;
	cv::filter2D(region, filteredImage, CV_32F, cornerKernel);
	
	// Find corner where value is max
	cv::Point maxP = likeliestCorner(filteredImage);
//	cv::minMaxLoc(filteredImage, NULL, NULL, NULL, &maxP);
	
	// Calculate subpixel position
	std::vector<cv::Point2f> sub = { maxP }; // vector needed for cornerSubPix
	cv::TermCriteria criteria = cv::TermCriteria( CV_TERMCRIT_EPS + CV_TERMCRIT_ITER, 20, 0.001 );
	cv::cornerSubPix( filteredImage, sub, cv::Size(3,3), cv::Size(-1,-1), criteria );
	return sub[0];
}

cv::Point EyeCorner::likeliestCorner(const cv::Mat &filtered) {
	static const int size = 4;
	static const int size2 = size * 2 + 1;
	cv::Mat mask = cv::Mat::ones(filtered.size(), CV_8UC1);
	
	cv::Point maxPos;
	float maxBrightness = 0;
	
	int i = 4; // find the 4 most likeliest corners
	while (i--) {
		cv::Point pos;
		cv::minMaxLoc(filtered, NULL, NULL, NULL, &pos, mask);
		
		cv::Rect2i area = cv::Rect2i(pos.x-size, pos.y-size, size2, size2);
		// subtract overlapping range
		if (area.x + area.width  > filtered.cols)  area.width  = filtered.cols - area.x;
		if (area.y + area.height > filtered.rows)  area.height = filtered.rows - area.y;
		if (area.x < 0) { area.width  += area.x; area.x = 0; }
		if (area.y < 0) { area.height += area.y; area.y = 0; }
		
		mask(area) = 0; // ignore this position in next iteration
		float areaBrightness = cv::mean(filtered(area))[0];
		
		if (areaBrightness > maxBrightness) {
			maxBrightness = areaBrightness;
			maxPos = pos;
		}
	}
	return maxPos;
}
