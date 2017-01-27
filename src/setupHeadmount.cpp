#include "setupHeadmount.h"

#include <fstream>

using namespace Setup;

cv::Rect arrangePoints(cv::Point a, cv::Point b) {
	cv::Rect r( fmin(a.x, b.x), fmin(a.y, b.y), 0, 0 );
	r.width = fmax(a.x, b.x) - r.x;
	r.height = fmax(a.y, b.y) - r.y;
	return r;
}

std::vector<cv::Point> loadFromFile(const char* path) {
	std::vector<cv::Point> points;
	if (path == NULL)
		return points;
	
	FILE *file = fopen(path, "r");
	cv::Point ltl, lbr, rtl, rbr;
	if ( file && fscanf(file, "[%d %d] [%d %d] - [%d %d] [%d %d]\n", &ltl.x, &ltl.y, &lbr.x, &lbr.y, &rtl.x, &rtl.y, &rbr.x, &rbr.y) ) {
		points.push_back(ltl);
		points.push_back(lbr);
		points.push_back(rtl);
		points.push_back(rbr);
	}
	fclose(file);
	return points;
}

void saveToFile(const char* path, std::vector<cv::Point> points) {
	if (path == NULL)
		return;
	
	std::ofstream outputStream(path);
	outputStream
	<< "[" << points[0].x << " " << points[0].y << "] "
	<< "[" << points[1].x << " " << points[1].y << "] - "
	<< "[" << points[2].x << " " << points[2].y << "] "
	<< "[" << points[3].x << " " << points[3].y << "]\n";
	outputStream.close();
}

RectPair pairFromPoints(std::vector<cv::Point> points) {
	float radiusL = cv::norm(points[0] - points[1]);
	float radiusR = cv::norm(points[2] - points[3]);
	cv::Rect l = cv::Rect(points[0].x - radiusL, points[0].y - radiusL, 2*radiusL, 2*radiusL);
	cv::Rect r = cv::Rect(points[2].x - radiusR, points[2].y - radiusR, 2*radiusR, 2*radiusR);
	
//	cv::Rect l = arrangePoints(points[0], points[1]);
//	cv::Rect r = arrangePoints(points[2], points[3]);
	if (l.x < r.x)
		return std::make_pair(l, r);
	else
		return std::make_pair(r, l);
}

void Headmount::mouseHandler(int event, int x, int y, int flags, void* param) {
	switch(event) {
		case CV_EVENT_LBUTTONUP:
			std::vector<cv::Point>* points = (std::vector<cv::Point>*)param;
			if (points->size() < 4)
				points->push_back(cv::Point(x,y));
			break;
	}
}

Headmount::Headmount(const cv::String window, const char* file) : windowName(window), savePath(file)
{
	if (file && strlen(file) > 2) // auto load any existing pos for video file
		userPoints = loadFromFile(savePath);
	
	if (userPoints.size() < 4) {
		setMouseCallback(window, mouseHandler, &userPoints);
	} else {
		superFastInit = true;
	}
}

bool Headmount::waitForInput(cv::Mat frame, RectPair *eyeRegion) {
	if (superFastInit) {
		*eyeRegion = pairFromPoints(userPoints);
		return true; // user setup complete
	}
	
	// User instructions
	cv::String infoText = "Mouse click to select eye regions (ESC undo)";
	if (userPoints.size() == 4)
		infoText = "Confirm selection with spacebar.";
	cv::putText(frame, infoText, cv::Point(10, frame.rows - 10), cv::FONT_HERSHEY_PLAIN, 2.0f, cv::Scalar(255,255,255));
	
	// Draw current selection
	for (int i = 0; i < userPoints.size(); i++) {
		circle(frame, userPoints[i], 3, 1234);
		
		if ((i % 2) == 0 && (i+1) < userPoints.size()) {
			float ptDistance = cv::norm(userPoints[i] - userPoints[i+1]);
			circle(frame, userPoints[i], ptDistance, 200);
		}
	}
	
	int key = cv::waitKey(300);
	switch (key) {
		case 27: // escape key, undo selection
			if (userPoints.size() == 0)
				exit(EXIT_FAILURE);
			userPoints.pop_back();
			break;
			
		case 'l': // L, load file
			userPoints = loadFromFile(savePath);
			break;
			
		case ' ': // spacebar, confirm selection
			if (userPoints.size() == 4) {
				saveToFile(savePath, userPoints);
				setMouseCallback(windowName, nullptr, NULL);
				*eyeRegion = pairFromPoints(userPoints);
				return true; // user setup complete
			}
			break;
	}
	return false; // continue setup
}
