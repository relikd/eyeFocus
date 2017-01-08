#include "findFace.h"
#include "constants.h"
#include "Debug.h"

using namespace Detector;

static Debug debugImage(MainWindow); // draw to main window

cv::Rect Face::findFace(cv::Mat &frame_gray) {
	//cvtColor( frame, frame_gray, CV_BGR2GRAY );
	//equalizeHist( frame_gray, frame_gray );
	//cv::pow(frame_gray, CV_64F, frame_gray);
	
	//-- Detect faces
	std::vector<cv::Rect> faces;
	face_cascade.detectMultiScale( frame_gray, faces, 1.1, 2, 0|CV_HAAR_SCALE_IMAGE|CV_HAAR_FIND_BIGGEST_OBJECT, cv::Size(150, 150) );
	
#if DEBUG_PLOT_ENABLED
	for( int i = 0; i < faces.size(); i++ ) {
		debugImage.addRectangle(faces[i]);
	}
#endif
	
	//-- Show what you got
	if (faces.size() > 0) {
		return faces[0];
	}
	return cv::Rect();
}

RectPair Face::findEyes(cv::Mat &faceROI) {
	if (kSmoothFaceImage) {
		double sigma = kSmoothFaceFactor * faceROI.cols;
		GaussianBlur( faceROI, faceROI, cv::Size( 0, 0 ), sigma);
	}
	
	//-- Find eye regions and draw them
	int eye_region_width = faceROI.cols * (kEyePercentWidth/100.0);
	int eye_region_height = faceROI.cols * (kEyePercentHeight/100.0);
	int eye_region_top = faceROI.rows * (kEyePercentTop/100.0);
	cv::Rect2f leftEyeRegion(faceROI.cols * (kEyePercentSide/100.0),
							 eye_region_top, eye_region_width, eye_region_height);
	cv::Rect2f rightEyeRegion(faceROI.cols - eye_region_width - faceROI.cols * (kEyePercentSide/100.0),
							  eye_region_top, eye_region_width, eye_region_height);
	
	return std::make_pair(leftEyeRegion, rightEyeRegion);
}
