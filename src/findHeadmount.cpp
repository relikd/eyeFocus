#include "findHeadmount.h"

using namespace Detector;

static std::vector<cv::Point> userPoints;

void mouseHandler(int event, int x, int y, int flags, void* param) {
	switch(event) {
		case CV_EVENT_LBUTTONUP:
			userPoints.push_back(cv::Point(x,y));
			break;
	}
}

cv::Rect arrangePoints(cv::Point a, cv::Point b) {
	cv::Rect r( fmin(a.x, b.x), fmin(a.y, b.y), 0, 0 );
	r.width = fmax(a.x, b.x) - r.x;
	r.height = fmax(a.y, b.y) - r.y;
	return r;
}

TwoEyes Headmount::askUserForInput(cv::VideoCapture cam, cv::String window) {
	cv::Mat frame;
	
	setMouseCallback(window, mouseHandler, NULL);
	
	while (userPoints.size() < 5) { // + 1 to allow redo with esc key
		
		cam.read(frame);
		if (frame.empty())
			continue;
		
		cv::flip(frame, frame, 1);
		
		for (int i = 0; i < userPoints.size(); i++)
		{
			circle(frame, userPoints[i], 3, 1234);
			
			if ((i % 2) == 0 && (i+1) < userPoints.size())
				rectangle(frame, userPoints[i], userPoints[i+1], 200);
		}
		imshow(window, frame);
		
		if( cv::waitKey(300) == 27 ) { // escape key
			if (userPoints.size() == 0)
				exit(EXIT_FAILURE);
			userPoints.pop_back();
		}
	}
	
	setMouseCallback(window, nullptr, NULL);
	
	cv::Rect l = arrangePoints(userPoints[0], userPoints[1]);
	cv::Rect r = arrangePoints(userPoints[2], userPoints[3]);
	if (l.x < r.x) {
		return std::make_pair(l, r);
	} else {
		return std::make_pair(r, l);
	}
}
