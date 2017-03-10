//
//  setupDualCam.cpp
//  eyeFocus
//
//  Created by Oleg Geier on 12/02/17.
//
//

#include "setupDualCam.h"
#include <thread>
#include "../Detector/FindPupil.h"
#include "../Helper/FrameReader.h"
#include "../Helper/LogWriter.h"
#include "../constants.h"
#include "../Estimate/estimateDistance.h"

using namespace Setup;

static int currentFocusLevel = 5;

//  ---------------------------------------------------------------
// |
// |  Save frames to disk
// |
//  ---------------------------------------------------------------

void writeCameraStreamToDisk(int cam) {
	cv::VideoCapture vc(cam);
	vc.set(CV_CAP_PROP_FRAME_WIDTH, 640);
	vc.set(CV_CAP_PROP_FRAME_HEIGHT, 480);
	vc.set(CV_CAP_PROP_FPS, 30);
	
	cv::Mat img;
	size_t f = 0;
	while (++f) {
		if (currentFocusLevel % 10 == 5) { // skip recording and reset counter
			f = 0;
			continue;
		}
		vc.read(img);
		char buffer[200];
		snprintf(buffer, 200*sizeof(char), "%d/%d/frame_%lu.jpg", cam, currentFocusLevel, f);
		// folder must exist, otherwise no output
		imwrite(buffer, img);
	}
}

void DualCam::writeStreamToDisk(int startFocusCM) {
	currentFocusLevel = startFocusCM + 5; // start with inactive phase
	
	bool bothInSync = false;
	std::thread a([&bothInSync]{
		while (!bothInSync) { /* nothing */ }
		writeCameraStreamToDisk(0);
	});
	std::thread b([&bothInSync]{
		while (!bothInSync) { /* nothing */ }
		writeCameraStreamToDisk(1);
	});
	printf("running..\n");
	bothInSync = true;
	
	cv::String window = "Press space to move 5cm nearer (ESC abort)";
	cv::namedWindow(window, CV_WINDOW_NORMAL);
	cv::moveWindow(window, 150, 100);
	imshow(window, cv::Mat::zeros(480, 640, CV_8UC1));
	
	while (true) {
		int key = cv::waitKey(100);
		if (key == 27) {
			exit(EXIT_SUCCESS);
		} else if (key == ' ') {
			currentFocusLevel -= 5; // each 5cm step is inactive phase
			printf("Level: %d\n", currentFocusLevel);
		}
	}
	a.join();
	b.join();
}

//  ---------------------------------------------------------------
// |
// |  Eye Tracking
// |
//  ---------------------------------------------------------------

//static const int blackThreshhold = 25;
//static const int offsetBottomPx = 107;
//
//void calculateDualCamEyeDistance(cv::Mat &frame, bool isLeft) {
//	cv::Point2i pt = cv::Point2i((isLeft ? frame.cols-1 : 0), frame.rows - offsetBottomPx);
//	while (pt.x >= 0 && pt.x < frame.cols) {
//		printf("%d ", frame.at<uchar>(pt));
//		frame.at<uchar>(pt) = 255;
//		(isLeft ? pt.x-- : pt.x++);
//	}
//	printf("\n");
//}

std::vector<float> points;
static const int betweenCamDistancePX = 930;//295; // 18.08mm

float distanceBetweenPoints(cv::Point2f left, cv::Point2f right, int camWidth) {
	return (camWidth - left.x) + betweenCamDistancePX + right.x;
}

bool setupFinished(cv::Mat &frame, cv::Point2f pLeft, cv::Point2f pRight) {
#if 0
	points.push_back(1);
	points.push_back(1);
	points.push_back(1);
	return true;
#endif
	cv::String infoText;
	switch (points.size()) {
		case 0: infoText = "Focus on 20 cm and press spacebar"; break;
		case 1: infoText = "Focus on 50 cm and press spacebar"; break;
		case 2: infoText = "Focus on 80 cm and press spacebar"; break;
		default: infoText = "Press spacebar to confirm"; break;
	}
	cv::putText(frame, infoText, cv::Point(10, frame.rows - 10), cv::FONT_HERSHEY_PLAIN, 2.0f, cv::Scalar(255,255,255));
	imshow("setup", frame);
	
	int key = cv::waitKey(30);
	switch (key) {
		case 27: // escape key, undo selection
			if (points.size() == 0)
				exit(EXIT_FAILURE);
			points.pop_back();
			break;
			
		case ' ': // spacebar, confirm selection
			if (points.size() == 3) {
				cv::destroyWindow("setup");
				return true; // user setup complete
			}
			points.push_back(distanceBetweenPoints(pLeft, pRight, frame.cols));
			break;
	}
	return false;
}

DualCam::DualCam(const char *path, const char* file) {
//	FrameReader fr[2] = {FrameReader(1), FrameReader(0)}; // USB Cam 1 & 2
	char one[300], two[300];
	if (path == NULL && file == NULL) {
		one[0] = '1'; one[1] = '\0'; // USB Cam 1 & 2
		two[0] = '0'; two[1] = '\0';
	} else {
		snprintf(one, 300*sizeof(char), "%s/1/%s", path, file);
		snprintf(two, 300*sizeof(char), "%s/0/%s", path, file);
	}
	FrameReader fr[2] = { FrameReader::initWithArgv(one), FrameReader::initWithArgv(two) };
	FindKalmanPupil pupilDetector[2];
	
	fr[0].readNext();
	fr[1].readNext();
	cv::Point2f nullPoint;
	
	cv::Rect2i crop = cv::Rect2i(0, 0, fr[0].frame.cols, fr[0].frame.rows);
	crop = cv::Rect2i(crop.width/6, crop.height/10, crop.width/1.5, crop.height/1.5);
	
	char pupilPosLogFile[1024];
	snprintf(pupilPosLogFile, 1024*sizeof(char), "%s/%s.pupilpos.csv", path, file);
	LogWriter log( pupilPosLogFile, "pLx,pLy,pRx,pRy,PupilDistance,cLx,cLy,cRx,cRy,CornerDistance\n" );
	
	
#if kEnableImageWindow
	cv::namedWindow("Distance", CV_WINDOW_NORMAL);
	cv::namedWindow("Cam 0", CV_WINDOW_NORMAL);
	cv::namedWindow("Cam 1", CV_WINDOW_NORMAL);
	cv::moveWindow("Distance", 370, 200);
	cv::moveWindow("Cam 0", 50, 100);
	cv::moveWindow("Cam 1", 700, 100);
#endif
	
	
	Estimate::Distance distEst;
	
	bool finishedYet = false;
	
	while (true) {
		cv::RotatedRect pupil[2];
		// Process both cams
		for (int i = 0; i < 2; i++) {
			if (fr[i].readNext()) {
				cv::Mat img = fr[i].frame;
				pupil[i] = pupilDetector[i].findSmoothed(img(crop), ElSe::find, crop.tl());
#if kEnableImageWindow
				ellipse(img, pupil[i], 1234);
				circle(img, pupil[i].center, 3, 1234);
#endif
				imshow((i==0?"Cam 0":"Cam 1"), img);
			} else {
				continue;
			}
		}
		
		cv::Mat blackFrame = cv::Mat::zeros(fr[0].frame.size(), CV_8UC1);
//		log.writePointPair(pupil[0].center, pupil[1].center + cv::Point2f(blackFrame.cols+betweenCamDistancePX,0), false);
//		log.writePointPair(nullPoint, nullPoint, true);
		
		if (finishedYet) {
			double est = distEst.estimate( distanceBetweenPoints(pupil[0].center, pupil[1].center, blackFrame.cols) );
			Estimate::Distance::drawOnFrame(blackFrame, est);
			imshow("Distance", blackFrame);
			if( cv::waitKey(10) == 27 ) // esc key
				exit(EXIT_SUCCESS);
		} else {
			finishedYet = setupFinished(blackFrame, pupil[0].center, pupil[1].center);
			if (finishedYet) {
				distEst.initialize(points, std::vector<int>{200, 500, 800});
				distEst.printEquation();
				distEst.printAccuracy(points[0], points[2], "accuracy_dist_est.csv");
			}
		}
	}
}
