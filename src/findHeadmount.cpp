#include "findHeadmount.h"

#include "opencv2/highgui/highgui.hpp"

#include "constants.h"

/*
bool lEyeSet = false;
bool rEyeSet = false;
cv::Rect eye_left, eye_right;
int eye_region_width = 300;
int eye_region_height = 250;
std::vector<cv::Point> userPoints;


void mouseHandler(int event, int x, int y, int flags, void* param) {
	switch(event) {
		case CV_EVENT_LBUTTONUP:
			userPoints.push_back(cv::Point(x,y));
			break;
	}
}

cv::Rect adjustedRect(cv::Point a, cv::Point b) {
	return cv::Rect(min(a.x, b.x), min(a.y, b.y), max(a.x, b.x) - min(a.x, b.x), max(a.y, b.y) - min(a.y, b.y));
}

void setupPhase(cv::Mat image) {
	cv::Mat debugFrame = image.clone();
	while (userPoints.size() < 5) { // one more to allow redo
		for (int i = 0; i < userPoints.size(); i++) {
			circle(debugFrame, userPoints[i], 3, 1234);
			if ((i % 2) == 0 && (i+1) < userPoints.size())
			rectangle(debugFrame, userPoints[i], userPoints[i+1], 200);
		}
		imshow(window_name, debugFrame);
		
		if( cv::waitKey(300) == 27 ) { // escape
			userPoints.pop_back();
			debugFrame = image.clone();
		}
	}
	cvSetMouseCallback(window_name.c_str(), nullptr, NULL);
	
	cv::Rect l = adjustedRect(userPoints[0], userPoints[1]);
	cv::Rect r = adjustedRect(userPoints[2], userPoints[3]);
	if (l.x < r.x) {
		eye_left = l;
		eye_right = r;
	} else {
		eye_left = r;
		eye_right = l;
	}
}


void detectAndDisplay( cv::Mat frame ) {
	
	std::vector<cv::Mat> rgbChannels(3);
	cv::split(frame, rgbChannels);
	cv::Mat frame_gray = rgbChannels[2];
	
	cv::Mat faceROI = frame_gray( Rect(0,0,frame.cols, frame.rows) );
	
	cv::Mat debugFace = faceROI;
	
	//-- Find Eye Centers
	cv::Point leftPupil = findEyeCenter(faceROI, eye_left, "Left Eye");
	cv::Point rightPupil = findEyeCenter(faceROI, eye_right, "Right Eye");
	
	// change eye centers to face coordinates
	rightPupil.x += eye_right.x;
	rightPupil.y += eye_right.y;
	leftPupil.x += eye_left.x;
	leftPupil.y += eye_left.y;
	// draw eye centers
	circle(debugFace, rightPupil, 3, 1234);
	circle(debugFace, leftPupil, 3, 1234);
	
	imshow(window_name, faceROI);
}
*/
