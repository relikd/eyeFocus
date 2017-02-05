#include "Debug.h"
#include <opencv2/highgui/highgui.hpp>

static cv::Mat debugImageStore[2]; // add more if needed

Debug::Debug(DebugStore s) : debugImage( debugImageStore[s] ) {}


inline cv::Point2f pointWithOffset(cv::Point2f &input, cv::Rect &offsetXY) {
	return cv::Point2f(input.x + offsetXY.x, input.y + offsetXY.y);
}

// Getter & Setter
cv::Mat& Debug::getImage() {
	return debugImage;
}

void Debug::setImage(cv::Mat &source) {
	source.copyTo( debugImage );
}

void Debug::display(cv::String window_name) {
	if (!debugImage.empty())
		imshow(window_name, debugImage);
}

// Plotting
void Debug::addRectangle(cv::Rect2f r, cv::Scalar color) {
	rectangle(debugImage, r, color);
}

void Debug::addCircle(cv::Point2f p, cv::Scalar color) {
	circle(debugImage, p, 3, color);
};
