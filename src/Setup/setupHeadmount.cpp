#include "setupHeadmount.h"
#include <fstream>
#include <opencv2/highgui/highgui.hpp>

#define USER_POINTS_NEEDED 6

using namespace Setup;

cv::Rect2i arrangePoints(cv::Point a, cv::Point b) {
	cv::Rect r( std::min(a.x, b.x), std::min(a.y, b.y), 0, 0 );
	r.width = std::max(a.x, b.x) - r.x;
	r.height = std::max(a.y, b.y) - r.y;
	return r;
}

cv::Rect2i rectFromPoint(cv::Point point, int size) {
	cv::Rect r = cv::Rect(point.x - size, point.y - size, 2*size, 2*size);
	if (r.y < 0) r.y = 0;
	return r;
}

std::vector<cv::Point> loadFromFile(const char* path) {
	std::vector<cv::Point> points;
	if (path == NULL)
		return points;
	
	FILE *file = fopen(path, "r");
	if ( file == NULL  )
		return points;
		
	cv::Point ltl, lbr, rtl, rbr;
	fscanf(file, "Eye Area:\n");
	if (fscanf(file, "[%d %d] [%d %d] - [%d %d] [%d %d]\n", &ltl.x, &ltl.y, &lbr.x, &lbr.y, &rtl.x, &rtl.y, &rbr.x, &rbr.y)) {
		points.push_back(ltl);
		points.push_back(lbr);
		points.push_back(rtl);
		points.push_back(rbr);
		cv::Point cl, cr;
		fscanf(file, "Eye Corner:\n");
		if (fscanf(file, "[%d %d] [%d %d]\n", &cl.x, &cl.y, &cr.x, &cr.y)) {
			if (cl.x > 0 && cl.y > 0)
				points.push_back(cl);
			if (cr.x > 0 && cr.y > 0)
				points.push_back(cr);
		}
	}
	fclose(file);
	return points;
}

void saveToFile(const char* path, std::vector<cv::Point> points) {
	if (path == NULL)
		return;
	
	std::ofstream outputStream(path);
	outputStream
	<< "Eye Area:\n"
	<< "[" << points[0].x << " " << points[0].y << "] "
	<< "[" << points[1].x << " " << points[1].y << "] - "
	<< "[" << points[2].x << " " << points[2].y << "] "
	<< "[" << points[3].x << " " << points[3].y << "]\n"
	<< "Eye Corner:\n"
	<< "[" << points[4].x << " " << points[4].y << "] "
	<< "[" << points[5].x << " " << points[5].y << "]\n";
	outputStream.close();
}

RectPair pairFromPoints(std::vector<cv::Point> points) {
	cv::Rect2i l = rectFromPoint(points[0], (int)cv::norm(points[0] - points[1]));
	cv::Rect2i r = rectFromPoint(points[2], (int)cv::norm(points[2] - points[3]));
	if (l.x < r.x)
		return std::make_pair(l, r);
	else
		return std::make_pair(r, l);
}

RectPair eyeCornerPair(std::vector<cv::Point> points) {
	return std::make_pair(rectFromPoint(points[4], kEyeCornerSearchArea), rectFromPoint(points[5], kEyeCornerSearchArea));
}

void Headmount::mouseHandler(int event, int x, int y, int flags, void* param) {
	switch(event) {
		case CV_EVENT_LBUTTONUP:
			std::vector<cv::Point>* points = (std::vector<cv::Point>*)param;
			if (points->size() < USER_POINTS_NEEDED)
				points->push_back(cv::Point(x,y));
			break;
	}
}

Headmount::Headmount(const cv::String window, const char* file) : windowName(window), savePath(file)
{
	if (file && strlen(file) > 2) // auto load any existing pos for video file
		userPoints = loadFromFile(savePath);
	
	if (userPoints.size() < USER_POINTS_NEEDED) {
		setMouseCallback(window, mouseHandler, &userPoints);
	} else {
		superFastInit = true;
	}
}

void Headmount::drawInstructionsAndUserSelection(cv::Mat frame) {
	// User instructions
	cv::String infoText = "Mouse click to select eye regions (ESC undo)";
	if (userPoints.size() == USER_POINTS_NEEDED)
		infoText = "Confirm selection with spacebar.";
	cv::putText(frame, infoText, cv::Point(10, frame.rows - 10), cv::FONT_HERSHEY_PLAIN, 2.0f, cv::Scalar(255,255,255));
	
	// Draw current selection
	for (int i = 0; i < userPoints.size(); i++) {
		circle(frame, userPoints[i], 3, 1234);
		
		if ((i % 2) == 0 && (i+1) < userPoints.size()) {
			if (i < 4) {
				int ptDistance = (int)cv::norm(userPoints[i] - userPoints[i+1]);
				circle(frame, userPoints[i], ptDistance, 200);
			} else {
				line(frame, userPoints[i], userPoints[i+1], 200);
				circle(frame, userPoints[i], 30, 200);
				circle(frame, userPoints[i+1], 30, 200);
			}
		}
	}
}

bool Headmount::waitForInput(cv::Mat frame, RectPair *eyeRegion, RectPair *eyeCornerRegion)
{
#if kFullsizeSingleEyeMode
	cv::Rect2i full = cv::Rect2i(0, 0, frame.cols, frame.rows);
	*eyeRegion = std::make_pair(full, full);
	*eyeCornerRegion = std::make_pair(full, full);
	return true;
#endif
	
	if (superFastInit) {
		*eyeRegion = pairFromPoints(userPoints);
		*eyeCornerRegion = eyeCornerPair(userPoints);
		return true; // user setup complete
	}
	
	drawInstructionsAndUserSelection(frame);
	
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
			if (userPoints.size() == USER_POINTS_NEEDED) {
				saveToFile(savePath, userPoints);
				setMouseCallback(windowName, nullptr, NULL);
				*eyeRegion = pairFromPoints(userPoints);
				*eyeCornerRegion = eyeCornerPair(userPoints);
				return true; // user setup complete
			}
			break;
	}
	return false; // continue setup
}