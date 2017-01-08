#include "setupHeadmount.h"

#include <fstream>

using namespace Setup;

static const char* preloadPosition = "../../../res/headmount_pos.txt";
static std::vector<cv::Point> userPoints;

void mouseHandler(int event, int x, int y, int flags, void* param) {
	switch(event) {
		case CV_EVENT_LBUTTONUP:
			if (userPoints.size() < 4)
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

void loadFromFile(const char* path) {
	FILE *file = fopen(path, "r");
	cv::Point ltl, lbr, rtl, rbr;
	if ( file && fscanf(file, "[%d %d] [%d %d] - [%d %d] [%d %d]\n", &ltl.x, &ltl.y, &lbr.x, &lbr.y, &rtl.x, &rtl.y, &rbr.x, &rbr.y) ) {
		userPoints.push_back(ltl);
		userPoints.push_back(lbr);
		userPoints.push_back(rtl);
		userPoints.push_back(rbr);
	}
	fclose(file);
}

void saveToFile(const char* path) {
	std::ofstream outputStream(path);
	outputStream
	<< "[" << userPoints[0].x << " " << userPoints[0].y << "] "
	<< "[" << userPoints[1].x << " " << userPoints[1].y << "] - "
	<< "[" << userPoints[2].x << " " << userPoints[2].y << "] "
	<< "[" << userPoints[3].x << " " << userPoints[3].y << "]\n";
	outputStream.close();
}

RectPair Headmount::askUserForInput(cv::VideoCapture cam, cv::String window) {
	cv::Mat frame;
	
	setMouseCallback(window, mouseHandler, NULL);
	
	while (true) {
		
		cam.read(frame);
		if (frame.empty())
			continue;
		
		cv::flip(frame, frame, 1);
		
		// User instructions
		cv::String infoText = "Mouse click to select eye regions (ESC undo)";
		if (userPoints.size() == 4)
			infoText = "Confirm selection with spacebar.";
		cv::putText(frame, infoText, cv::Point(10, frame.rows - 10), cv::FONT_HERSHEY_PLAIN, 2.0f, cv::Scalar(255,255,255));
		
		// Draw current selection
		for (int i = 0; i < userPoints.size(); i++) {
			circle(frame, userPoints[i], 3, 1234);
			
			if ((i % 2) == 0 && (i+1) < userPoints.size())
				rectangle(frame, userPoints[i], userPoints[i+1], 200);
		}
		imshow(window, frame);
		
		
		int key = cv::waitKey(300);
		if( key == 27 ) // escape key, undo selection
		{
			if (userPoints.size() == 0)
				exit(EXIT_FAILURE);
			// Undo selection
			userPoints.pop_back();
		}
		else if ( key == 'l' )  // L, load file
		{
			loadFromFile(preloadPosition);
		}
		else if ( key == ' ' ) // spacebar, confirm selection
		{
			if (userPoints.size() == 4)
				break;
		}
	}
	
	saveToFile(preloadPosition);
	
	setMouseCallback(window, nullptr, NULL);
	
	
	cv::Rect l = arrangePoints(userPoints[0], userPoints[1]);
	cv::Rect r = arrangePoints(userPoints[2], userPoints[3]);
	if (l.x < r.x) {
		return std::make_pair(l, r);
	} else {
		return std::make_pair(r, l);
	}
}
