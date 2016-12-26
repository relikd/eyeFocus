#include "findSkin.h"

using namespace Detector;

cv::Mat skinCrCbHist = cv::Mat::zeros(cv::Size(256, 256), CV_8UC1);

Skin::Skin() {
	ellipse(skinCrCbHist, cv::Point(113, 155.6), cv::Size(23.4, 15.2),
			43.0, 0.0, 360.0, cv::Scalar(255, 255, 255), -1);
}

cv::Mat Skin::findSkin (cv::Mat &frame) {
	cv::Mat input;
	cv::Mat output = cv::Mat(frame.rows,frame.cols, CV_8U);
	
	cvtColor(frame, input, CV_BGR2YCrCb);
	
	for (int y = 0; y < input.rows; ++y) {
		const cv::Vec3b *Mr = input.ptr<cv::Vec3b>(y);
//		uchar *Or = output.ptr<uchar>(y);
		cv::Vec3b *Or = frame.ptr<cv::Vec3b>(y);
		for (int x = 0; x < input.cols; ++x) {
			cv::Vec3b ycrcb = Mr[x];
//			Or[x] = (skinCrCbHist.at<uchar>(ycrcb[1], ycrcb[2]) > 0) ? 255 : 0;
			if(skinCrCbHist.at<uchar>(ycrcb[1], ycrcb[2]) == 0) {
				Or[x] = cv::Vec3b(0,0,0);
			}
		}
	}
	return output;
}
