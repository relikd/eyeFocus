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
#include <thread>

static int currentFocusLevel = 85;

#if !kCameraIsHeadmounted
//-- Note, either copy these two files from opencv/d1ata/haarscascades to your current folder, or change these locations
static Detector::Face faceDetector = Detector::Face("res/haarcascade_frontalface_alt.xml");
#endif

void startNormalTracking(FrameReader &fr);
void startSingleEyeTracking(FrameReader &fr);
void startDualCamTracking();

void writeCameraStreamToDisk(int cam) {
	cv::VideoCapture vc(cam);
	vc.set(CV_CAP_PROP_FRAME_WIDTH, 640);
	vc.set(CV_CAP_PROP_FRAME_HEIGHT, 480);
	vc.set(CV_CAP_PROP_FPS, 60);
	
	cv::Mat img;
	size_t f = 0;
	while (++f) {
		if (currentFocusLevel % 10 == 5) {
			f = 0;
			continue;
		}
		vc.read(img);
		char buffer[200];
		snprintf(buffer, 200*sizeof(char), "%d/%d/frame_%lu.jpg", cam, currentFocusLevel, f);
		imwrite(buffer, img);
	}
}

void startMultiThreadingDualCamRead() {
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
	
	cv::String window = "Press space to move 5cm nearer";
	cv::namedWindow(window, CV_WINDOW_NORMAL);
	cv::moveWindow(window, 50, 100);
	imshow(window, cv::Mat::zeros(480, 640, CV_8UC1));
	
	while (true) {
		int key = cv::waitKey(100);
		if (key == 27) {
			exit(EXIT_SUCCESS);
		} else if (key == ' ') {
			currentFocusLevel -= 5;
			printf("Level: %d\n", currentFocusLevel);
		}
	}
	a.join();
	b.join();
}

int main( int argc, const char** argv ) {
	if (argc != 2) {
		fputs("Missing argument value. Pass either [path to video file] or [camera index].\n\n", stderr);
		return EXIT_SUCCESS;
	}
	
#if 0
	startMultiThreadingDualCamRead();
	return EXIT_SUCCESS;
#endif
	
#if kFullsizeDualCamMode
	startDualCamTracking();
	return EXIT_SUCCESS;
#endif
	
	// create Video Capture from calling argument
	FrameReader fr = FrameReader::initWithArgv(argv[1]);
	fr.downScaling = 2; // reduce size for GoPro
	
#if kFullsizeSingleEyeMode
	startSingleEyeTracking(fr); // will never return
	return EXIT_SUCCESS;
#endif
	
	startNormalTracking(fr);
	return EXIT_SUCCESS;
}

//  ---------------------------------------------------------------
// |
// |  Drawing on frame
// |
//  ---------------------------------------------------------------

void drawDistance(cv::Mat &frame, int distance) {
	char strEst[6];
	snprintf(strEst, 6*sizeof(char), "%dcm", distance);
	cv::putText(frame, strEst, cv::Point(frame.cols - 220, frame.rows - 10), cv::FONT_HERSHEY_PLAIN, 5.0f, cv::Scalar(255,255,255));
}

void drawDebugPlot(cv::Mat &frame, cv::Rect2i box, cv::RotatedRect pupil) {
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


// ################################################################
// #
// #  Different Tracker Types
// #
// ################################################################

//  ---------------------------------------------------------------
// |
// |  Single Eye Tracking Mode (1 Camera, 1 Pupil)
// |
//  ---------------------------------------------------------------

/** Single pupil is filling the complete cam image */
void startSingleEyeTracking(FrameReader &fr) {
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
// |  Dual Cam Tracking Mode (2 Cameras, 1 Pupil each)
// |
//  ---------------------------------------------------------------

/** Same as startSingleEyeTracking() but with two cameras */
void startDualCamTracking() {
	FrameReader fr[2] = {FrameReader(1), FrameReader(0)}; // USB Cam 1 & 2
	FindKalmanPupil pupilDetector[2];
	
	fr[0].readNext();
	cv::Point2f nullPoint;
	
	cv::Rect2i crop = cv::Rect2i(0, 0, fr[0].frame.cols, fr[0].frame.rows);
	crop = cv::Rect2i(crop.width/4, crop.height/4, crop.width/2, crop.height/2);
	
	char pupilPosLogFile[1024];
	snprintf(pupilPosLogFile, 1024*sizeof(char), "%s.pupilpos.csv", fr[0].filePath);
	LogWriter log( pupilPosLogFile, "pLx,pLy,pRx,pRy,PupilDistance,cLx,cLy,cRx,cRy,CornerDistance\n" );
	
	
#if kEnableImageWindow
	cv::namedWindow("Cam 0", CV_WINDOW_NORMAL);
	cv::namedWindow("Cam 1", CV_WINDOW_NORMAL);
	cv::moveWindow("Cam 0", 50, 100);
	cv::moveWindow("Cam 1", 700, 100);
#endif
	
	while (true) {
		cv::RotatedRect pupil[2];
		// Process both cams
		for (int i = 0; i < 2; i++) {
			if (fr[i].readNext()) {
				cv::Mat img = fr[i].frame;
				pupil[i] = pupilDetector[i].findSmoothed(img(crop), ElSe::find, crop.tl());
#if kEnableImageWindow
				drawDebugPlot(img, crop, pupil[i]);
#endif
				imshow((i==0?"Cam 0":"Cam 1"), img);
			}
		}
		log.writePointPair(pupil[0].center, pupil[1].center, false);
		log.writePointPair(nullPoint, nullPoint, true);
		
		if( cv::waitKey(10) == 27 ) // esc key
			exit(EXIT_SUCCESS);
	}
}

//  ---------------------------------------------------------------
// |
// |  Normal Tracking Mode (1 Camera, 2 Pupils) [optional: Face Detection]
// |
//  ---------------------------------------------------------------

void initHeadmount(FrameReader &fr, cv::Rect2i eyeBox[2], cv::Rect2i eyeCorner[2]) {
	char savePath[1024] = "cam.eyepos.txt";
	if (fr.isVideoFile)
		snprintf(savePath, 1024*sizeof(char), "%s.eyepos.txt", fr.filePath);
	Setup::Headmount setupHead(fr, savePath);
	eyeBox[0] = setupHead.leftEyeBox;
	eyeBox[1] = setupHead.rightEyeBox;
	eyeCorner[0] = setupHead.leftEyeCorner;
	eyeCorner[1] = setupHead.rightEyeCorner;
}

void startNormalTracking(FrameReader &fr) {
	cv::Rect2i eyeBox[2];
	cv::Rect2i eyeCorner[2];
	
#if kCameraIsHeadmounted // Manually select eye region
	initHeadmount(fr, eyeBox, eyeCorner);
#endif
	
	
#if kEnableImageWindow
	cv::namedWindow(fr.filePath, CV_WINDOW_NORMAL);
	cv::moveWindow(fr.filePath, 400, 100);
#endif
	
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
		int est = distEst.estimate(pupil[0], pupil[1], corner[0], corner[1], false);
		drawDistance(img, est);
		imshow(fr.filePath, img);
		
		if( cv::waitKey(10) == 27 ) // esc key
			exit(EXIT_SUCCESS);
#endif
	}
}
