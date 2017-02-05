#ifndef TrackingDebug_h
#define TrackingDebug_h

#include <opencv2/imgproc/imgproc.hpp>

enum DebugStore {
	MainWindow = 0,
	EyeImage = 1
};

#define DEBUG_PLOT_ENABLED 0

class Debug {
	cv::Mat &debugImage;
	
public:
	Debug(DebugStore s);
	
	cv::Mat& getImage();
	void setImage(cv::Mat &source);
	void display(cv::String window_name);
	
	void addRectangle(cv::Rect2f r, cv::Scalar color = 1234);
	void addCircle(cv::Point2f p, cv::Scalar color = 1234);
};



#endif /* TrackingDebug_h */