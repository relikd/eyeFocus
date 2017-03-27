//
//  trackingSingleCam.h
//  eyeFocus
//
//  Created by Oleg Geier on 27/03/17.
//
//

#include "trackingSingleCam.h"

#include "../Estimate/estimateDistance.h"

using namespace Tracking;

void SingleCam::start(FrameReader &fr) {
	FindKalmanPupil tracker;
	// Single Eye Calibration
	init(fr, &tracker);
	// use full video size for eye tracking, adjust to eg. remove edge
	cv::Rect2i clip = cv::Rect2i(0, 0, fr.frame.cols, fr.frame.rows);
	
	Estimate::Distance distEst;
	float pplDist[3] = {cm20.x, cm50.x, cm80.x};
	int focusDist[3] = {200, 500, 800};
	distEst.initialize(3, pplDist, focusDist);
	
	while ( fr.readNext() ) {
		cv::RotatedRect point = tracker.findSmoothed(fr.frame(clip), ElSe::find, clip.tl());
		circle(fr.frame, point.center, 3, 1234);
		ellipse(fr.frame, point, 1234);
		double est = distEst.estimate(point.center.x);
		
		Estimate::Distance::drawOnFrame(fr.frame, est);
		imshow(fr.filePath, fr.frame);
		
		if( cv::waitKey(10) == 27 ) // esc key
			exit(EXIT_SUCCESS);
	}
}

namespace NS_SingleCam {
	cv::Point2f averagePoint(std::vector<cv::Point2f> points) {
		cv::Point2f sum = cv::Point2f(0,0);
		for (cv::Point2f &p : points)
			sum += p;
		sum.x /= points.size();
		sum.y /= points.size();
		return sum;
	}
	cv::Point2f findCenter(cv::Mat image, FindKalmanPupil* tracker) {
		// make search area smaller, find pupil, then add offset
		cv::Rect2i rectWithoutEdge(image.cols*0.1, image.rows*0.1, image.cols*0.8, image.rows*0.8);
		cv::RotatedRect rr = tracker->findSmoothed(image(rectWithoutEdge), ElSe::find);
		rr.center.x += image.cols*0.1;
		rr.center.y += image.rows*0.1;
		return rr.center;
	}
}

void SingleCam::init(FrameReader &fr, FindKalmanPupil* tracker) {
	
	std::vector<cv::Point2f> pupilAverage;
	int internalSetIndex = 0;
	
	cv::namedWindow(window_setup_single_eye, CV_WINDOW_NORMAL);
	cv::moveWindow(window_setup_single_eye, 400, 100);
	
	while (fr.readNext()) {
		pupilAverage.push_back(NS_SingleCam::findCenter(fr.frame, tracker));
		
		while (pupilAverage.size() > 10) // limit average to the last x points
			pupilAverage.erase(pupilAverage.begin());
		
		cv::String infoText;
		switch (internalSetIndex) {
			case 0: infoText = "Focus on 20 cm and press spacebar"; break;
			case 1: infoText = "Focus on 50 cm and press spacebar"; break;
			case 2: infoText = "Focus on 80 cm and press spacebar"; break;
			default: infoText = "Press spacebar to confirm"; break;
		}
		cv::putText(fr.frame, infoText, cv::Point(10, fr.frame.rows - 10), cv::FONT_HERSHEY_PLAIN, 2.0f, cv::Scalar(255,255,255));
		
		imshow(window_setup_single_eye, fr.frame);
		
		int key = cv::waitKey(30);
		switch (key) {
			case 27: // escape key, undo selection
				if (internalSetIndex == 0)
					exit(EXIT_FAILURE);
				--internalSetIndex;
				break;
				
			case ' ': // spacebar, confirm selection
				if (internalSetIndex == 3) {
					cv::destroyWindow(window_setup_single_eye);
					return; // user setup complete
				}
				
				cv::Point2f avg = NS_SingleCam::averagePoint(pupilAverage);
				switch (internalSetIndex) {
					case 0: cm20 = avg; break;
					case 1: cm50 = avg; break;
					default: cm80 = avg; break;
				}
				++internalSetIndex;
				break;
		}
	}
}

