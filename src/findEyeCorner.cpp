#include "findEyeCorner.h"
//#include <opencv2/highgui/highgui.hpp> // debug window

using namespace Detector;

cv::Point2f EyeCorner::findByAvgColor(cv::Mat region, bool left) {
	double avgValue = cv::sum(region)[0] / (region.cols * region.rows);
	cv::Mat tmp;
	cv::GaussianBlur(region, tmp, cv::Size(9,9), 0, 0);
	cv::threshold(tmp, tmp, avgValue * 0.95, 255, cv::THRESH_BINARY);
	cv::GaussianBlur(tmp, tmp, cv::Size(9,9), 0, 0);
	
	return (left ? kflc : kfrc).smoothedPosition( find(tmp, left, left) );
	
//	cv::Canny(tmp, tmp, 0.1, 0.1);
	
//	int y = tmp.rows/2;
//	int x = tmp.cols/2 + (isLeftEye ? -10 : 10);
//	int val = tmp.at<int>(y,x);
//	while (x < tmp.cols && x >= 0) {
//		(isLeftEye ? x++ : x--);
//		if (tmp.at<int>(y,x) != val)
//			break;
//	}
	//imshow("sdfs", tmp);
	//cv::waitKey();
	//cv::circle(tmp, cv::Point(x,y), 3, 200);
//	return cv::Point2f(x,y);
	
//
//	std::vector<std::vector<cv::Point> > contours;
//	std::vector<cv::Vec4i> hierarchy;
//	cv::findContours( tmp, contours, hierarchy, CV_RETR_CCOMP, CV_CHAIN_APPROX_SIMPLE );
//
//	cv::Point2f corner;
//	cv::RotatedRect r;
//	for (std::vector<cv::Point> vpt : contours) {
//		if (vpt.size() > 50) {
//			r = fitEllipse(vpt);
//
//			float piAngle = (r.angle / 180.0) * M_PI;
//
//			if ((isLeftEye && (fabsf(r.angle - 45) < 15.0 || fabsf(r.angle - 225) < 15.0)) ||
//				(!isLeftEye && (fabsf(r.angle - 135) < 15.0 || fabsf(r.angle - 315) < 15.0)))
//			{
//				float dist = r.size.width / 2.0;
//				corner = r.center + cv::Point2f(cos(piAngle) * dist, sin(piAngle) * dist);
//			} else {
//				float dist = r.size.height / 2.0;
//				corner = r.center + cv::Point2f(cos(piAngle+M_PI_2) * dist, sin(piAngle+M_PI_2) * dist);
//			}
//			if (corner.inside(cv::Rect(0,0,tmp.cols,tmp.rows))) {
//				break;
//			}
//		}
//	}
//	printf("angle: %f, width: %f, height: %f\n", r.angle, r.size.width, r.size.height);
//	printf("%1.3f, %1.3f\n", r.center.x, r.center.y);
//	printf("%1.3f, %1.3f\n", corner.x, corner.y);
//	cv::ellipse(tmp, r, 200);
//	cv::circle(tmp, corner, 3, 200);
//	return tmp;
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
	
	return cv::Point2f(sizeRegion.width / 2.0F + maxP2.x / 10.0F,
					   sizeRegion.height / 2.0F + maxP2.y / 10.0F);
}
