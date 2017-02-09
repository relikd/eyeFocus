#include <opencv2/highgui/highgui.hpp>
#include "constants.h"
#include "Setup/setupEyeCoordinateSpace.h"
#include "Setup/setupHeadmount.h"
#include "Setup/setupSingleEye.h"
#include "Detector/findFace.h"
#include "Detector/findEyeCorner.h"
#include "Estimate/estimateDistance.h"
#include "Helper/FrameReader.h"
#include "Helper/LogWriter.h"


#if !kCameraIsHeadmounted
//-- Note, either copy these two files from opencv/d1ata/haarscascades to your current folder, or change these locations
static Detector::Face faceDetector = Detector::Face("res/haarcascade_frontalface_alt.xml");
#endif

void initHeadmount(FrameReader fr, cv::Rect2i eyeBox[2], cv::Rect2i eyeCorner[2]);
void startSingleEyeTracking(FrameReader fr);
void drawDistance(cv::Mat frame, int distance);
void drawDebugPlot(cv::Mat frame, cv::Rect2i box, cv::RotatedRect pupil);

int main( int argc, const char** argv ) {
	if (argc != 2) {
		fputs("Missing argument value. Pass either [path to video file] or [camera index].\n\n", stderr);
		return EXIT_SUCCESS;
	}
	
	// create Video Capture from calling argument
	FrameReader fr = FrameReader::initWithArgv(argv[1]);
	fr.downScaling = 2; // reduce size for GoPro
	
#if kFullsizeSingleEyeMode
	startSingleEyeTracking(fr); // will never return
#endif
	
	cv::Rect2i eyeBox[2];
	cv::Rect2i eyeCorner[2];

#if kCameraIsHeadmounted // Manually select eye region
	initHeadmount(fr, eyeBox, eyeCorner);
#endif
	
	
	cv::namedWindow(fr.filePath, CV_WINDOW_NORMAL);
	cv::moveWindow(fr.filePath, 400, 100);
	
	char pupilPosLogFile[1024];
	snprintf(pupilPosLogFile, 1024*sizeof(char), "%s.pupilpos.csv", fr.filePath);
	LogWriter log( pupilPosLogFile, "pLx,pLy,pRx,pRy,PupilDistance,cLx,cLy,cRx,cRy,CornerDistance\n" );
	
	FindKalmanPupil pupilDetector[2];
	Detector::EyeCorner cornerDetector[2]; cornerDetector[1].flipKernelToRightCorner();
	Estimate::Distance distEst("estimate.cfg");
	
	while ( fr.readNext() ) {
		cv::Mat img = fr.frame;
		
#if !kCameraIsHeadmounted
		cv::Rect2i face_r = faceDetector.find(fr.frame, &eyeBox[0], &eyeBox[1]);
		rectangle(img, face_r, 200);
#endif
		
		if (kSmoothFaceImage) {
			double sigma = kSmoothFaceFactor * img.cols;
			GaussianBlur( img, img, cv::Size( 0, 0 ), sigma);
		}
		
		cv::RotatedRect pupil[2];
		cv::Point2f corner[2];
		for (int i = 0; i < 2; i++) {
			pupil[i] = pupilDetector[i].findSmoothed(img(eyeBox[i]), ElSe::find, eyeBox[i].tl());
			corner[i] = cornerDetector[i].findByAvgColor(img(eyeCorner[i]), eyeCorner[i].tl());
			drawMarker(img, corner[i], 200);
			drawDebugPlot(img, eyeBox[i], pupil[i]);
		}
		log.writePointPair(pupil[0].center, pupil[1].center, false);
		log.writePointPair(corner[0], corner[1], true);
		
		// Estimate distance
		int est = distEst.estimate(pupil[0], pupil[1], corner[0], corner[1], false);
		drawDistance(fr.frame, est);
		
		imshow(fr.filePath, img);
		if( cv::waitKey(10) == 27 ) // esc key
			return EXIT_SUCCESS;
	}
	return EXIT_SUCCESS;
}

//  ---------------------------------------------------------------
// |
// |  Different Tracker-Type Related Functions
// |
//  ---------------------------------------------------------------

void initHeadmount(FrameReader fr, cv::Rect2i eyeBox[2], cv::Rect2i eyeCorner[2]) {
	char savePath[1024] = "cam.eyepos.txt";
	if (fr.isVideoFile)
		snprintf(savePath, 1024*sizeof(char), "%s.eyepos.txt", fr.filePath);
	Setup::Headmount setupHead(fr, savePath);
	eyeBox[0] = setupHead.leftEyeBox;
	eyeBox[1] = setupHead.rightEyeBox;
	eyeCorner[0] = setupHead.leftEyeCorner;
	eyeCorner[1] = setupHead.rightEyeCorner;
}

/** Single pupil is filling the complete cam image */
void startSingleEyeTracking(FrameReader fr) {
	FindKalmanPupil tracker;
	// Single Eye Calibration
	Setup::SingleEye singleEye = Setup::SingleEye(fr, &tracker);
	// use full video size for eye tracking, adjust to eg. remove edge
	cv::Rect2i clip = cv::Rect2i(0, 0, fr.frame.cols, fr.frame.rows);
	
	while ( fr.readNext() ) {
		cv::RotatedRect point = tracker.findSmoothed(fr.frame(clip), ElSe::find, clip.tl());
		circle(fr.frame, point.center, 3, 1234);
		ellipse(fr.frame, point, 1234);
		int est = Estimate::Distance::singlePupilHorizontal(point.center.x, singleEye.cm20.x, singleEye.cm50.x, singleEye.cm80.x);
		
		drawDistance(fr.frame, est);
		imshow(fr.filePath, fr.frame);
		
		if( cv::waitKey(10) == 27 ) // esc key
			exit(EXIT_SUCCESS);
	}
}

//  ---------------------------------------------------------------
// |
// |  Drawing on frame
// |
//  ---------------------------------------------------------------

void drawDistance(cv::Mat frame, int distance) {
	char strEst[6];
	snprintf(strEst, 6*sizeof(char), "%dcm", distance);
	cv::putText(frame, strEst, cv::Point(frame.cols - 220, frame.rows - 10), cv::FONT_HERSHEY_PLAIN, 5.0f, cv::Scalar(255,255,255));
}

void drawDebugPlot(cv::Mat frame, cv::Rect2i box, cv::RotatedRect pupil) {
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
