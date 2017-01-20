#include <opencv2/highgui/highgui.hpp>
#include "opencv2/video/tracking.hpp"

#include "Debug.h"
#include "constants.h"

#include "findFace.h"
#include "findEyeCenter.h"
#include "findEyeCorner.h"
#include "setupEyeCoordinateSpace.h"
#include "setupHeadmount.h"

/** Global variables */
#define CAMERA_SOURCE 0 //-1

static Debug debugMain(MainWindow);
static Debug debugEye(EyeImage);
static cv::KalmanFilter KFL(4,2,0); // left pupil
static cv::KalmanFilter KFR(4,2,0); // right pupil


//-- Note, either copy these two files from opencv/data/haarscascades to your current folder, or change these locations
static Detector::Face detectFace = Detector::Face("../../../res/haarcascade_frontalface_alt.xml");
static Detector::EyeCenter detectCenter = Detector::EyeCenter();
static Detector::EyeCorner detectCorner = Detector::EyeCorner();

PointPair findPupils( cv::Mat faceROI, RectPair eyes, cv::Point2f offset );

int main( int argc, const char** argv )
{
	cv::VideoCapture capture( CAMERA_SOURCE );
	if( !capture.isOpened() )
		return EXIT_FAILURE;
	
	cv::namedWindow(window_name_main,CV_WINDOW_NORMAL);
	cv::moveWindow(window_name_main, 400, 100);
	cv::namedWindow(window_name_face,CV_WINDOW_AUTOSIZE);
	cv::moveWindow(window_name_face, 100, 100);
	cv::namedWindow(window_name_right_eye,CV_WINDOW_NORMAL);
	cv::moveWindow(window_name_right_eye, 10, 600);
	cv::namedWindow(window_name_left_eye,CV_WINDOW_NORMAL);
	cv::moveWindow(window_name_left_eye, 10, 800);
	
	
//	cv::namedWindow("Test1",CV_WINDOW_NORMAL);
//	cv::moveWindow("Test1", 10, 400);
//	cv::namedWindow("Test2",CV_WINDOW_NORMAL);
//	cv::moveWindow("Test2", 10, 500);
	
	
	// Init Kalman filter
	if (kUseKalmanFilter) {
		KFL.transitionMatrix = (cv::Mat_<float>(4, 4) << 1,0,1,0,   0,1,0,1,  0,0,1,0,  0,0,0,1);
		KFR.transitionMatrix = (cv::Mat_<float>(4, 4) << 1,0,1,0,   0,1,0,1,  0,0,1,0,  0,0,0,1);
		
		setIdentity(KFL.measurementMatrix);
		setIdentity(KFL.processNoiseCov, cv::Scalar::all(kKalmanProcessError));
		setIdentity(KFL.measurementNoiseCov, cv::Scalar::all(kKalmanMeasureError)); // error in measurement
		setIdentity(KFL.errorCovPost, cv::Scalar::all(kKalmanInitialError));
		setIdentity(KFR.measurementMatrix);
		setIdentity(KFR.processNoiseCov, cv::Scalar::all(kKalmanProcessError));
		setIdentity(KFR.measurementNoiseCov, cv::Scalar::all(kKalmanMeasureError));
		setIdentity(KFR.errorCovPost, cv::Scalar::all(kKalmanInitialError));
	}
	
	RectPair eyes;
	cv::Mat frame;
	cv::Mat faceROI;
	cv::Point2f headOffset;
	std::vector<cv::Mat> rgbChannels(3);
	
	if (kCameraIsHeadmounted) {
		eyes = Setup::Headmount::askUserForInput(capture, window_name_main);
	}
	
	Setup::EyeCoordinateSpace ecs = Setup::EyeCoordinateSpace();
	bool continueECSSetup = false;
	
	while ( true ) {
		capture.read(frame);
		
		if( frame.empty() ) {
			printf(" --(!) No captured frame -- Break!\n");
			continue;
		}
		
		// mirror it
		cv::flip(frame, frame, 1);
		debugMain.setImage(frame);
		// get gray image from blue channel
		cv::split(frame, rgbChannels);
		cv::Mat frame_gray = rgbChannels[2];
		
		// -- Get eye region
		if (kCameraIsHeadmounted) {
			// whole image is 'face' area for eye detection
			faceROI = frame_gray;
		} else {
			// Apply the classifier to the frame
			cv::Rect face_r = detectFace.findFace(frame_gray);
			headOffset = face_r.tl();
			faceROI = frame_gray(face_r);
			eyes = detectFace.findEyes(faceROI);
		}
		
		PointPair pupils = findPupils( faceROI, eyes, headOffset );
		
		if (continueECSSetup) {
			continueECSSetup = ecs.waitForInput(debugMain.getImage(), eyes, pupils, headOffset);
		} else {
			int c = cv::waitKey(10);
			if( (char)c == 27 ) { break; } // esc key
			if( (char)c == 'f' ) {
				imwrite("frame.png",frame);
			}
		}
		
		debugMain.addCircle(pupils.first);
		debugMain.addCircle(pupils.second);
		debugMain.display(window_name_main);
	}
	
	return EXIT_SUCCESS;
}



cv::Point2f findPupil( cv::Mat &faceImage, cv::Rect2f &eyeRegion, bool isLeftEye )
{
	if (eyeRegion.area()) {
		cv::Point2f pupil = detectCenter.findEyeCenter(faceImage, eyeRegion, (isLeftEye ? window_name_left_eye : window_name_right_eye) );
		
		if (pupil.x < 5 || pupil.y < 5) {
			if (kUseKalmanFilter) {
				// Reuse last point if no pupil found (eg. eyelid closed)
				cv::Mat prevPos = (isLeftEye ? KFL : KFR).statePre;
				pupil = cv::Point2f(prevPos.at<float>(0), prevPos.at<float>(1));
			} else {
				// pupil position can be != 0 since it is upscaled in the previous process
				// we have to reset it to actual 0,0 to indicate a 'not found'
				pupil = cv::Point2f();
			}
		}
		
		if (kUseKalmanFilter) {
			// 1. Prediction
			cv::Mat prediction = (isLeftEye ? KFL : KFR).predict();
			cv::Point2f predictPt(prediction.at<float>(0),prediction.at<float>(1));
			
			// 2. Get Measured Data
			cv::Mat_<float> measurement(2,1);
			measurement(0) = pupil.x;
			measurement(1) = pupil.y;
			
			// 3. Update Status
			cv::Mat estimated = (isLeftEye ? KFL : KFR).correct(measurement);
			cv::Point2f statePt(estimated.at<float>(0),estimated.at<float>(1));
			
			pupil = statePt;
		}
		
		
#if DEBUG_PLOT_ENABLED
		// get tiled eye region
		//  .-----------.
		//  |___________|
		//  |      |    |
		//  | L    *  R |  // * = pupil
		//  |______|____|
		//  |           |
		//  '-----------'
		cv::Rect2f leftRegion(eyeRegion.x, eyeRegion.y, pupil.x, eyeRegion.height / 2);
		leftRegion.y += leftRegion.height / 2;
		
		cv::Rect2f rightRegion(leftRegion);
		rightRegion.x += pupil.x;
		rightRegion.width = eyeRegion.width - pupil.x;
		
		if (kEnableEyeCorner) {
			cv::Point2f leftCorner = detectCorner.find(faceImage(leftRegion), isLeftEye, true);
			cv::Point2f rightCorner = detectCorner.find(faceImage(rightRegion), isLeftEye, false);
			debugEye.addCircle( leftCorner + leftRegion.tl() , 200 );
			debugEye.addCircle( rightCorner + rightRegion.tl() , 200 );
		}
		
		// draw eye region
		debugEye.addRectangle(eyeRegion);
		
		// draw tiled eye box
		debugEye.addRectangle(leftRegion, 200);
		debugEye.addRectangle(rightRegion, 200);
		
		// draw eye center
		debugEye.addCircle(pupil + eyeRegion.tl());
#endif
		
		return pupil + eyeRegion.tl(); // add offset
	}
	return cv::Point2f();
}

PointPair findPupils( cv::Mat faceROI, RectPair eyes, cv::Point2f offset ) {
	debugEye.setImage(faceROI);
	
	//-- Find Eye Centers
	cv::Point2f leftPupil = findPupil( faceROI, eyes.first, true ) + offset;
	cv::Point2f rightPupil = findPupil( faceROI, eyes.second, false ) + offset;
	
	float eyeDistance = cv::norm(leftPupil - rightPupil);
	printf("L[%1.1f,%1.1f] - R[%1.1f,%1.1f] (distance: %1.1f)\n", leftPupil.x, leftPupil.y, rightPupil.x, rightPupil.y, eyeDistance);
	
	//cv::Rect roi( cv::Point( 0, 0 ), faceROI.size());
	//cv::Mat destinationROI = debugFace( roi );
	//faceROI.copyTo( destinationROI );
	
	debugEye.display(window_name_face);
	
	return std::make_pair(leftPupil, rightPupil);
}

