#include <opencv2/highgui/highgui.hpp>

#include "Debug.h"
#include "constants.h"

#include "findFace.h"
#include "findHeadmount.h"
#include "findEyeCenter.h"
#include "findEyeCorner.h"

/** Global variables */
#define CAMERA_SOURCE 0 //-1

static Debug debugMain(MainWindow);
static Debug debugEye(EyeImage);

//-- Note, either copy these two files from opencv/data/haarscascades to your current folder, or change these locations
static Detector::Face detectFace = Detector::Face("../../../res/haarcascade_frontalface_alt.xml");
static Detector::EyeCenter detectCenter = Detector::EyeCenter();
static Detector::EyeCorner detectCorner = Detector::EyeCorner();

typedef std::pair<cv::Point2f, cv::Point2f> TwoPupils;
TwoPupils findPupils( cv::Mat faceROI, TwoEyes eyes, cv::Point2f offset );

int main( int argc, const char** argv )
{
	cv::VideoCapture capture( CAMERA_SOURCE );
	if( !capture.isOpened() )
		return EXIT_FAILURE;
	
	cv::namedWindow(window_name_main,CV_WINDOW_NORMAL);
	cv::moveWindow(window_name_main, 400, 100);
	cv::namedWindow(window_name_right_eye,CV_WINDOW_NORMAL);
	cv::moveWindow(window_name_right_eye, 10, 600);
	cv::namedWindow(window_name_left_eye,CV_WINDOW_NORMAL);
	cv::moveWindow(window_name_left_eye, 10, 800);
	
	if ( !kCameraIsHeadmounted ) {
		cv::namedWindow(window_name_face,CV_WINDOW_AUTOSIZE);
		cv::moveWindow(window_name_face, 10, 100);
	}
	
	TwoEyes eyes;
	cv::Mat frame;
	cv::Mat faceROI;
	cv::Point2f headOffset;
	std::vector<cv::Mat> rgbChannels(3);
	
	if (kCameraIsHeadmounted) {
		eyes = Detector::Headmount::askUserForInput(capture, window_name_main);
	}
	
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
		
		TwoPupils pupils = findPupils( faceROI, eyes, headOffset );
		
		debugMain.addCircle(pupils.first);
		debugMain.addCircle(pupils.second);
		debugMain.display(window_name_main);
		
		int c = cv::waitKey(10);
		if( (char)c == 27 ) { break; } // esc key
		if( (char)c == 'f' ) {
			imwrite("frame.png",frame);
		}
	}
	
	return EXIT_SUCCESS;
}



cv::Point2f findPupil( cv::Mat &faceImage, cv::Rect2f &eyeRegion, bool isLeftEye )
{
	if (eyeRegion.area()) {
		cv::Point2f pupil = detectCenter.findEyeCenter(faceImage, eyeRegion, (isLeftEye ? window_name_left_eye : window_name_right_eye) );
		
#if DEBUG_PLOT_ENABLED
		if ( !kCameraIsHeadmounted ) {
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
		}
#endif
		
		return pupil + eyeRegion.tl(); // add offset
	}
	return cv::Point2f();
}


TwoPupils findPupils( cv::Mat faceROI, TwoEyes eyes, cv::Point2f offset ) {
	if ( !kCameraIsHeadmounted ) {
		debugEye.setImage(faceROI);
	}
	
	//-- Find Eye Centers
	cv::Point2f leftPupil = findPupil( faceROI, eyes.first, true ) + offset;
	cv::Point2f rightPupil = findPupil( faceROI, eyes.second, false ) + offset;
	
	printf("L[%1.1f,%1.1f] - R[%1.1f,%1.1f]\n", leftPupil.x, leftPupil.y, rightPupil.x, rightPupil.y);
	
	//cv::Rect roi( cv::Point( 0, 0 ), faceROI.size());
	//cv::Mat destinationROI = debugFace( roi );
	//faceROI.copyTo( destinationROI );
	
	if ( !kCameraIsHeadmounted ) {
		debugEye.display(window_name_face);
	}
	
	return std::make_pair(leftPupil, rightPupil);
}

