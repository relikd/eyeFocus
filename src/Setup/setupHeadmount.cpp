//
//  setupHeadmount.cpp
//  eyeFocus
//
//  Created by Oleg Geier on 26/12/16.
//
//

#include "setupHeadmount.h"
#include <opencv2/highgui/highgui.hpp>
#include <fstream>
#include "../constants.h"

using namespace Setup;

namespace NS_Headmount {
	static const int points_needed = 6;
	
	cv::Rect2i arrangePoints(cv::Point a, cv::Point b) {
		cv::Rect r( std::min(a.x, b.x), std::min(a.y, b.y), 0, 0 );
		r.width = std::max(a.x, b.x) - r.x;
		r.height = std::max(a.y, b.y) - r.y;
		return r;
	}
	
	cv::Rect2i rectWithRadius(cv::Point point, int size) {
		cv::Rect r = cv::Rect(point.x - size, point.y - size, 2*size, 2*size);
		if (r.y < 0) r.y = 0;
		return r;
	}
	
	std::vector<cv::Point> loadFromFile(const char* path, int scalingFactor) {
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
		for (cv::Point &p : points) {
			p.x /= scalingFactor;
			p.y /= scalingFactor;
		}
		fclose(file);
		return points;
	}
	
	std::string pointToString(cv::Point2i p, int scale = 1) {
		char buffer[20];
		snprintf(buffer, 20*sizeof(char), "[%d %d]", p.x * scale, p.y * scale);
		return std::string(buffer);
	}
	
	void saveToFile(const char* path, std::vector<cv::Point> points, int scale) {
		if (path == NULL)
			return;
		
		std::ofstream outputStream(path);
		outputStream
		<< "Eye Area:\n"
		<< pointToString(points[0], scale) << " " << pointToString(points[1], scale) << " - "
		<< pointToString(points[2], scale) << " " << pointToString(points[3], scale) << "\n"
		<< "Eye Corner:\n"
		<< pointToString(points[4], scale) << " " << pointToString(points[5], scale) << "\n";
		outputStream.close();
	}
	
	void mouseHandler(int event, int x, int y, int flags, void* param) {
		switch(event) {
			case CV_EVENT_LBUTTONUP:
				std::vector<cv::Point>* points = (std::vector<cv::Point>*)param;
				if (points->size() < points_needed)
					points->push_back(cv::Point(x,y));
				break;
		}
	}
	
	void drawInstructionsForSelection(cv::Mat frame, std::vector<cv::Point> points) {
		// User instructions
		cv::String infoText = "Mouse click to select eye regions (ESC undo)";
		if (points.size() == points_needed)
			infoText = "Confirm selection with spacebar.";
		cv::putText(frame, infoText, cv::Point(10, frame.rows - 10), cv::FONT_HERSHEY_PLAIN, 2.0f, cv::Scalar(255,255,255));
		
		// Draw current selection
		for (int i = 0; i < points.size(); i++) {
			circle(frame, points[i], 3, 1234);
			
			if ((i % 2) == 0 && (i+1) < points.size()) {
				if (i < 4) {
					int ptDistance = (int)cv::norm(points[i] - points[i+1]);
					circle(frame, points[i], ptDistance, 200);
				} else {
					line(frame, points[i], points[i+1], 200);
					circle(frame, points[i], 30, 200);
					circle(frame, points[i+1], 30, 200);
				}
			}
		}
	}
}


Headmount::Headmount(FrameReader &fr, const char* savePath)
{
	std::vector<cv::Point> userPoints;
	// load previous eye box
	userPoints = NS_Headmount::loadFromFile(savePath, fr.downScaling);
	
	// if file exists and is valid, return the previous boxes
	if (userPoints.size() == NS_Headmount::points_needed)
		goto return_eye_box;
	
	
	cv::namedWindow(window_setup_headmount, CV_WINDOW_NORMAL);
	cv::moveWindow(window_setup_headmount, 400, 100);
	
	setMouseCallback(window_setup_headmount, NS_Headmount::mouseHandler, &userPoints);
	
	while (fr.readNext()) {
		NS_Headmount::drawInstructionsForSelection(fr.frame, userPoints);
		imshow(window_setup_headmount, fr.frame);
		
		int key = cv::waitKey(300);
		switch (key) {
			case 27: // escape key, undo selection
				if (userPoints.size() == 0)
					exit(EXIT_FAILURE);
				userPoints.pop_back();
				break;
				
			case 'l': // L, load file
				userPoints = NS_Headmount::loadFromFile(savePath, fr.downScaling);
				break;
				
			case ' ': // spacebar, confirm selection
				if (userPoints.size() == NS_Headmount::points_needed) {
					NS_Headmount::saveToFile(savePath, userPoints, fr.downScaling);
					setMouseCallback(window_setup_headmount, nullptr, NULL);
					goto return_eye_box;
				}
				break;
		}
	}
	
return_eye_box:
	cv::destroyWindow(window_setup_headmount);
	
	cv::Rect2i l = NS_Headmount::rectWithRadius(userPoints[0], (int)cv::norm(userPoints[0] - userPoints[1]));
	cv::Rect2i r = NS_Headmount::rectWithRadius(userPoints[2], (int)cv::norm(userPoints[2] - userPoints[3]));
	leftEyeBox  = (l.x < r.x ? l : r);
	rightEyeBox = (l.x < r.x ? r : l); // flip left and right
	leftEyeCorner = NS_Headmount::rectWithRadius(userPoints[4], kEyeCornerSearchArea);
	rightEyeCorner = NS_Headmount::rectWithRadius(userPoints[5], kEyeCornerSearchArea);
}
