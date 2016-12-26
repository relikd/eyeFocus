#include "Debug.h"
#include <opencv2/highgui/highgui.hpp>

static cv::Mat debugImageStore[2]; // add more if needed

Debug::Debug(DebugStore s) : debugImage( debugImageStore[s] ) {}


inline cv::Point2f pointWithOffset(cv::Point2f &input, cv::Rect &offsetXY) {
	return cv::Point2f(input.x + offsetXY.x, input.y + offsetXY.y);
}

//  ---------------------------------------------------------------
// |
// |  Getter / Setter
// |
//  ---------------------------------------------------------------
cv::Mat& Debug::getImage() {
	return debugImage;
}

void Debug::setImage(cv::Mat &source) {
#if DEBUG_PLOT_ENABLED
	source.copyTo( debugImage );
#endif
}

void Debug::display(std::string window_name) {
#if DEBUG_PLOT_ENABLED
	if (!debugImage.empty())
		imshow(window_name, debugImage);
#endif
}


//  ---------------------------------------------------------------
// |
// |  Plot
// |
//  ---------------------------------------------------------------
void Debug::addRectangle(cv::Rect r, cv::Scalar color) {
#if DEBUG_PLOT_ENABLED
	rectangle(debugImage, r, color);
#endif
}

void Debug::addCircle(cv::Point p, cv::Scalar color) {
#if DEBUG_PLOT_ENABLED
	circle(debugImage, p, 3, color);
#endif
};
