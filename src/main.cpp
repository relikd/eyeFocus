#include <opencv2/highgui/highgui.hpp>

#include "Debug.h"
#include "constants.h"

#include "findFace.h"
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


void detectAndDisplay( cv::Mat frame );

int main( int argc, const char** argv ) {
	// I make an attempt at supporting both 2.x and 3.x OpenCV
#if CV_MAJOR_VERSION < 3
	CvCapture* capture = cvCaptureFromCAM( CAMERA_SOURCE );
	if( !capture )
		return EXIT_FAILURE;
#else
	cv::VideoCapture capture( CAMERA_SOURCE );
	if( !capture.isOpened() )
		return EXIT_FAILURE;
#endif
	
	cv::namedWindow(window_name_main,CV_WINDOW_NORMAL);
	cv::moveWindow(window_name_main, 400, 100);
	cv::namedWindow(window_name_face,CV_WINDOW_AUTOSIZE);
	cv::moveWindow(window_name_face, 10, 100);
	cv::namedWindow(window_name_right_eye,CV_WINDOW_NORMAL);
	cv::moveWindow(window_name_right_eye, 10, 600);
	cv::namedWindow(window_name_left_eye,CV_WINDOW_NORMAL);
	cv::moveWindow(window_name_left_eye, 10, 800);
	
	cv::Mat frame;
	while( true ) {
		
#if CV_MAJOR_VERSION < 3
		frame = cvQueryFrame( capture );
#else
		capture.read(frame);
#endif
		
		// mirror it
		cv::flip(frame, frame, 1);
		debugMain.setImage(frame);
		
		// Apply the classifier to the frame
		if( !frame.empty() ) {
			detectAndDisplay( frame );
		} else {
			printf(" --(!) No captured frame -- Break!\n");
			continue;
		}
		debugMain.display(window_name_main);
		
		int c = cv::waitKey(10);
		if( (char)c == 'c' ) { break; }
		if( (char)c == 'f' ) {
			imwrite("frame.png",frame);
		}
	}
	
	return EXIT_SUCCESS;
}

inline cv::Point2f pointWithOffset(cv::Point2f &input, cv::Rect &offsetXY) {
	return cv::Point2f(input.x + offsetXY.x, input.y + offsetXY.y);
}

cv::Point2f findPupil( cv::Mat &faceImage, cv::Rect2f &eyeRegion, bool isLeftEye )
{
	if (eyeRegion.area()) {
		cv::Point2f pupil = detectCenter.findEyeCenter(faceImage, eyeRegion, (isLeftEye ? window_name_left_eye : window_name_right_eye) );
		
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

void detectAndDisplay( cv::Mat frame )
{
	std::vector<cv::Mat> rgbChannels(3);
	cv::split(frame, rgbChannels);
	cv::Mat frame_gray = rgbChannels[2];
	cv::Mat faceROI;
	TwoEyes eyes;
	
	if (kCameraIsHeadmounted) {
		faceROI = frame_gray; // whole image is search area for eye detection
	} else {
		cv::Rect face_r = detectFace.findFace(frame_gray);
		faceROI = frame_gray(face_r);
		eyes = detectFace.findEyes(faceROI);
	}
	
	debugEye.setImage(faceROI);
	
	//-- Find Eye Centers
	cv::Point2f leftPupil = findPupil( faceROI, eyes.first, true ); // left
	cv::Point2f rightPupil = findPupil( faceROI, eyes.second, false ); // right
	
	printf("L[%1.1f,%1.1f] - R[%1.1f,%1.1f]\n", leftPupil.x, leftPupil.y, rightPupil.x, rightPupil.y);
	
	debugEye.display(window_name_face);
	
	//	cv::Rect roi( cv::Point( 0, 0 ), faceROI.size());
	//	cv::Mat destinationROI = debugFace( roi );
	//	faceROI.copyTo( destinationROI );
}
