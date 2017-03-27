//
//  setupDualCam.cpp
//  eyeFocus
//
//  Created by Oleg Geier on 12/02/17.
//
//

#include "setupDualCam.h"
#include "../Detector/FindPupil.h"
#include "../Helper/FrameReader.h"
#include "../Helper/LogWriter.h"
#include "../Helper/FileIO.h"
#include "../constants.h"
#include "../Estimate/estimateDistance.h"

using namespace Setup;

std::vector<float> pplDistancePoints;
std::vector<int> focalPoints;
int currentFocusDistance = 200;

static const int betweenCamDistancePX = 930; // 36.47mm
inline float dualCamDistanceBetweenPoints(cv::Point2f left, cv::Point2f right, int camWidth) {
	return (camWidth - left.x) + betweenCamDistancePX + right.x;
}

bool dualCamCalibrationFile(const char* path, bool write = false) {
	if (path) {
		FILE* file = FileIO::openFile(path, write);
		if (file) {
			if (write) {
				for (int i = 0; i < focalPoints.size(); i++) {
					fprintf(file, "%d;%1.32f\n", focalPoints[i], pplDistancePoints[i]);
				}
			} else {
				int d;
				float f;
				while (fscanf(file, "%d;%f\n", &d, &f) > 0) {
					pplDistancePoints.push_back(f);
					focalPoints.push_back(d);
				}
			}
			fclose(file);
			return true;
		}
	}
	return false;
}

inline void dualCamInitEstimator(Estimate::Distance &estimator) {
	estimator.initialize(pplDistancePoints, focalPoints);
	estimator.printEquation();
}

inline void dualCamWritePlot(int winx, cv::String window, cv::Mat plot, cv::String filename = "") {
	cv::namedWindow(window, CV_WINDOW_NORMAL);
	cv::moveWindow(window, winx, 400);
	imshow(window, plot);
	if (!filename.empty()) imwrite(filename, plot);
}

void dualCamGraphOutput(Estimate::Distance &estimator) {
	int min = 99999, max = 0;
	int count = focalPoints.size();
	std::vector<cv::Point2f> measured;
	for (int i = 0; i < count; i++) {
		float pplDist = pplDistancePoints[i];
		if (min > pplDist) min = pplDist;
		if (max < pplDist) max = pplDist;
		measured.push_back(cv::Point2f(pplDist, focalPoints[i]));
	}
	if (count > 0) {
		cv::Mat plotFx = estimator.graphFunction(min-10, max+10, measured);
		cv::Mat plotErr = estimator.graphUncertainty(min-10, max+10, "graph_uncertainty.csv");
		dualCamWritePlot(50, "Graph: Distance estimation f(x)", plotFx, "graph_estimation_function.jpg");
		dualCamWritePlot(700, "Graph: Uncertainty for 1px", plotErr, "graph_uncertainty.jpg");
		cv::waitKey(0);
		cv::destroyWindow("Graph: Distance estimation f(x)");
		cv::destroyWindow("Graph: Uncertainty for 1px");
	}
}

bool dualCamClearMeasurement() {
	focalPoints.clear();
	pplDistancePoints.clear();
	return false; // for 'finishedYet' variable
}

bool dualCamSetup(cv::Mat &frame, cv::Point2f pLeft, cv::Point2f pRight) {
	cv::putText(frame, FileIO::str("Focus on %d cm (%lu)", currentFocusDistance/10, focalPoints.size()),
				cv::Point(10, frame.rows - 10), cv::FONT_HERSHEY_PLAIN, 2.0f, cv::Scalar(255,255,255));
	imshow("Distance", frame);
	
	int key = cv::waitKey(30);
	switch (key) {
		case 13: // return, confirm selection
			//cv::destroyWindow("setup");
			return true; // user setup complete
			
		case 27: // escape key, undo selection
			if (focalPoints.size() == 0)
				exit(EXIT_FAILURE);
			focalPoints.pop_back();
			pplDistancePoints.pop_back();
			break;
			
		case 'r':
			dualCamClearMeasurement();
			break;
			
		case ' ': // spacebar, measure point
			pplDistancePoints.push_back( dualCamDistanceBetweenPoints(pLeft, pRight, frame.cols) );
			focalPoints.push_back( currentFocusDistance );
			break;
			
		case '9':
//			currentFocusDistance = 6000; // inf: 6m
//			break;
		case '0': case '1': case '2':
		case '3': case '4': case '5':
		case '6': case '7': case '8':
			currentFocusDistance = (key-48)*100;
			break;
	}
	return false;
}

DualCam::DualCam(const char *path, const char* file) {
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
	
	LogWriter log( FileIO::str("%s/%s.pupilpos.csv", path, file).c_str(),
				  "pLx,pLy,pRx,pRy,PupilDistance,cLx,cLy,cRx,cRy,CornerDistance\n" );
	
#if kEnableImageWindow
	cv::namedWindow("Distance", CV_WINDOW_NORMAL);
	cv::namedWindow("Cam 0", CV_WINDOW_NORMAL);
	cv::namedWindow("Cam 1", CV_WINDOW_NORMAL);
	cv::moveWindow("Distance", 370, 200);
	cv::moveWindow("Cam 0", 50, 100);
	cv::moveWindow("Cam 1", 700, 100);
#endif
	
	
	Estimate::Distance distEst;
	
	bool finishedYet = dualCamCalibrationFile("dualcam_calib.txt"); // read
	if (finishedYet)
		dualCamInitEstimator(distEst);
	
	while (true) {
		bool bothCamsEmpty = true;
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
				bothCamsEmpty = false;
			} else {
				continue;
			}
		}
		if (bothCamsEmpty) return;
		
		cv::Mat blackFrame = cv::Mat::zeros(fr[0].frame.size(), CV_8UC1);
		log.writePointPair(pupil[0].center, pupil[1].center + cv::Point2f(blackFrame.cols+betweenCamDistancePX,0), false);
		log.writePointPair(nullPoint, nullPoint, true);
		
		if (finishedYet) {
			double est = distEst.estimate( dualCamDistanceBetweenPoints(pupil[0].center, pupil[1].center, blackFrame.cols) );
			Estimate::Distance::drawOnFrame(blackFrame, est);
			imshow("Distance", blackFrame);
			int key = cv::waitKey(10);
			if (key == 27)  exit(EXIT_SUCCESS); // esc key
			if (key == 'g') dualCamGraphOutput(distEst);
			if (key == 'r') finishedYet = dualCamClearMeasurement();
		} else {
			finishedYet = dualCamSetup(blackFrame, pupil[0].center, pupil[1].center);
			if (finishedYet) {
				dualCamInitEstimator(distEst);
				dualCamCalibrationFile("dualcam_calib.txt", true); // write
			}
		}
	}
}
