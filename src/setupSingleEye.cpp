#include "setupSingleEye.hpp"
#include <opencv2/highgui/highgui.hpp>
#include <cmath>

using namespace Setup;

cv::Point2f averagePoint(std::vector<cv::Point2f> points) {
	cv::Point2f sum = cv::Point2f(0,0);
	for (cv::Point2f &p : points)
		sum += p;
	sum.x /= points.size();
	sum.y /= points.size();
	return sum;
}

int SingleEye::bestEstimate(cv::Point2f pupil) {
	double lowerRange = std::pow(cm50.x / cm20.x, 1/30.0);
	double upperRange = std::pow(cm80.x / cm50.x, 1/30.0);
	
	float initLower = cm20.x * std::pow(lowerRange, -10); // start at 10cm
	float initUpper = cm50.x;
	
	int bestMatchLower = 0;
	int bestMatchUpper = 0;
	
	float diff = 999;
	do {
		float newDiff = std::fabs(initLower - pupil.x);
		if (newDiff > diff) {
			return 10 + bestMatchLower - 1;
		}
		diff = newDiff;
		initLower *= lowerRange;
	} while (++bestMatchLower < 40);
	
	diff = 999;
	
	do {
		float newDiff = std::fabs(initUpper - pupil.x);
		if (newDiff > diff) {
			return 50 + bestMatchUpper - 1;
		}
		diff = newDiff;
		initUpper *= upperRange;
	} while (++bestMatchUpper < 30);
	
	return 100;
}

bool SingleEye::waitForInput(cv::Mat frame, cv::Point2f pupil) {
	pupilAverage.push_back(pupil);
	while (pupilAverage.size() > 10) // limit average to the last x points
		pupilAverage.erase(pupilAverage.begin());
	
	cv::String infoText;
	switch (internalSetIndex) {
		case 0: infoText = "Focus on 20 cm and press spacebar"; break;
		case 1: infoText = "Focus on 50 cm and press spacebar"; break;
		case 2: infoText = "Focus on 80 cm and press spacebar"; break;
		default: infoText = "Press spacebar to confirm"; break;
	}
	cv::putText(frame, infoText, cv::Point(10, frame.rows - 10), cv::FONT_HERSHEY_PLAIN, 2.0f, cv::Scalar(255,255,255));
	
	int key = cv::waitKey(30);
	switch (key) {
		case 27: // escape key, undo selection
			if (internalSetIndex == 0)
				exit(EXIT_FAILURE);
			--internalSetIndex;
			break;
			
		case ' ': // spacebar, confirm selection
			if (internalSetIndex == 3) {
				return true; // user setup complete
			}
			
			cv::Point2f avg = averagePoint(pupilAverage);
			switch (internalSetIndex) {
				case 0: cm20 = avg; break;
				case 1: cm50 = avg; break;
				default: cm80 = avg; break;
			}
			++internalSetIndex;
			break;
	}
	return false; // continue setup
}
