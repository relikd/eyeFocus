#ifndef TrackingDebug_h
#define TrackingDebug_h

#include <opencv2/imgproc/imgproc.hpp>

typedef enum {
	MainWindow = 0,
	EyeImage = 1
} DebugStore;

#define DEBUG_PLOT_ENABLED 1

class Debug {
	cv::Mat &debugImage;
	
public:
	Debug(DebugStore s);
	
	cv::Mat& getImage();
	void setImage(cv::Mat &source);
	void display(std::string window_name);
	
	void addRectangle(cv::Rect r, cv::Scalar color = 1234);
	void addCircle(cv::Point p, cv::Scalar color = 1234);
};



#endif /* TrackingDebug_h */
