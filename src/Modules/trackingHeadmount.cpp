//
//  trackingHeadmount.cpp
//  eyeFocus
//
//  Created by Oleg Geier on 27/03/17.
//
//

#include "trackingHeadmount.h"
#include "../Setup/setupHeadmount.h"
#include "../Detector/findEyeCorner.h"
#include "../Detector/findPupil.h"
#include "../Helper/LogWriter.h"

using namespace Tracking;

void Headmount::manuallySelectEyeRegion(FrameReader &fr) {
	char savePath[1024] = "cam.eyepos.txt";
	if (fr.isVideoFile)
		snprintf(savePath, 1024*sizeof(char), "%s.eyepos.txt", fr.filePath);
	Setup::Headmount setupHead(fr, savePath);
	eyeBox[0] = setupHead.leftEyeBox;
	eyeBox[1] = setupHead.rightEyeBox;
	eyeCorner[0] = setupHead.leftEyeCorner;
	eyeCorner[1] = setupHead.rightEyeCorner;
}

void Headmount::start(FrameReader &fr, bool findFace) {
	if (findFace)
		faceDetector.load("res/haarcascade_frontalface_alt.xml");
	else
		manuallySelectEyeRegion(fr);
	
#if kEnableImageWindow
	cv::namedWindow(fr.filePath, CV_WINDOW_NORMAL);
	cv::moveWindow(fr.filePath, 400, 100);
#endif
	
	LogWriter log( FileIO::str("%s.pupilpos.csv", fr.filePath).c_str(),
				  "pLx,pLy,pRx,pRy,PupilDistance,cLx,cLy,cRx,cRy,CornerDistance\n" );
	
	FindKalmanPupil pupilDetector[2];
	Detector::EyeCorner cornerDetector[2]; cornerDetector[1].flipKernelToRightCorner();
//	Estimate::Distance distEst("estimate.cfg");
	
	while ( fr.readNext() ) {
		cv::Mat img = fr.frame;
		
		if (findFace) {
			cv::Rect2i face_r = faceDetector.find(fr.frame, &eyeBox[0], &eyeBox[1]);
			rectangle(img, face_r, 200);
		}
	
		if (kSmoothFaceImage) {
			double sigma = kSmoothFaceFactor * img.cols;
			GaussianBlur( img, img, cv::Size( 0, 0 ), sigma);
		}
		
		cv::RotatedRect pupil[2];
		cv::Point2f corner[2];
		for (int i = 0; i < 2; i++) {
			pupil[i] = pupilDetector[i].findSmoothed(img(eyeBox[i]), Timm::find, eyeBox[i].tl());
			corner[i] = cornerDetector[i].findByAvgColor(img(eyeCorner[i]), eyeCorner[i].tl());
#if kEnableImageWindow
			drawMarker(img, corner[i], 200);
			drawDebugPlot(img, eyeBox[i], pupil[i]);
#endif
		}
		log.writePointPair(pupil[0].center, pupil[1].center, false);
		log.writePointPair(corner[0], corner[1], true);
		
#if kEnableImageWindow
		// Estimate distance
//		int est = distEst.estimate(pupil[0], pupil[1], corner[0], corner[1], false);
//		Estimate::Distance::drawOnFrame(img, est);
		imshow(fr.filePath, img);
		
		if( cv::waitKey(10) == 27 ) // esc key
			exit(EXIT_SUCCESS);
#endif
	}
}

//  ---------------------------------------------------------------
// |
// |  Drawing on frame
// |
//  ---------------------------------------------------------------

void Headmount::drawDebugPlot(cv::Mat &frame, cv::Rect2i box, cv::RotatedRect pupil) {
#if 0
	// get tiled eye region
	//  .-----------.
	//  |___________|
	//  |      |    |
	//  | L    *  R |  // * = pupil
	//  |______|____|
	//  |           |
	//  '-----------'
	cv::Rect2f leftRegion(box.x, box.y, pupil.center.x - box.x, box.height / 2);
	leftRegion.y += leftRegion.height / 2;
	
	cv::Rect2f rightRegion(leftRegion);
	rightRegion.x += leftRegion.width;
	rightRegion.width = box.width - leftRegion.width;
	
	// draw eye region
	rectangle(frame, box, 1234);
	
	// draw tiled eye box
	rectangle(frame, leftRegion, 200);
	rectangle(frame, rightRegion, 200);
#endif
	
	// draw eye center
	ellipse(frame, pupil, 1234);
	circle(frame, pupil.center, 3, 1234);
}

