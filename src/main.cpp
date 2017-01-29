#include <opencv2/highgui/highgui.hpp>
#include "constants.h"
#include "findFace.h"
#include "setupEyeCoordinateSpace.h"
#include "setupHeadmount.h"
#include "findPupils.hpp"

/** Global variables */
static bool sourceIsImageFile;

#if !kCameraIsHeadmounted
//-- Note, either copy these two files from opencv/data/haarscascades to your current folder, or change these locations
static Detector::Face detectFace = Detector::Face("res/haarcascade_frontalface_alt.xml");
#endif


enum SetupPhase {
	SetupHeadmount,
	SetupEyeCoordinateSpace,
	SetupComplete
};

cv::VideoCapture initProgram(const char* path) {
	cv::namedWindow(window_name_main,CV_WINDOW_NORMAL);
	cv::moveWindow(window_name_main, 400, 100);
#if DEBUG_PLOT_ENABLED
	cv::namedWindow(window_name_face,CV_WINDOW_AUTOSIZE);
	cv::moveWindow(window_name_face, 100, 100);
#endif
	cv::namedWindow(window_name_right_eye,CV_WINDOW_NORMAL);
	cv::moveWindow(window_name_right_eye, 10, 600);
	cv::namedWindow(window_name_left_eye,CV_WINDOW_NORMAL);
	cv::moveWindow(window_name_left_eye, 10, 800);
	
	sourceIsImageFile = (strlen(path) > 2); // string is not an index number
	
	// create Video Capture from calling argument
	if (sourceIsImageFile) {
		return cv::VideoCapture( path );
	} else {
		long index = 0;
		strtol(path, NULL, 10);
		return cv::VideoCapture( index );
	}
}

bool grabDownscaledGreyFrame(cv::VideoCapture capture, cv::Mat *frame) {
	cv::Mat img;
	capture.read(img);
	
	if( img.empty() ) {
		if (sourceIsImageFile)
			exit(EXIT_SUCCESS); // exit on EOF
		fputs(" --(!) No captured frame -- Break!\n", stderr);
		*frame = cv::Mat::zeros(640, 480, CV_8UC1);
		return false;
	}
	
	cv::resize(img, img, cv::Size(img.cols/2, img.rows/2));
	cv::flip(img, img, 1); // mirror it
	
	// get gray image from blue channel
	std::vector<cv::Mat> rgbChannels(3);
	cv::split(img, rgbChannels);
	*frame = rgbChannels[2];
	return true;
}

int main( int argc, const char** argv )
{
	if (argc != 2) {
		fputs("Missing argument value. Pass either [path to video file] or [camera index].\n\n", stderr);
		return EXIT_SUCCESS;
	}
	
	cv::VideoCapture capture = initProgram( argv[1] );
	if( !capture.isOpened() )
		return EXIT_FAILURE;
	
	SetupPhase state = SetupPhase::SetupComplete;
	
#if kSetupEyeCoordinateSpace
	Setup::EyeCoordinateSpace setupECS = Setup::EyeCoordinateSpace();
	state = SetupPhase::SetupEyeCoordinateSpace;
#endif
	
#if kCameraIsHeadmounted
	char headPosFile[1024];
	strcpy(headPosFile, argv[1]);
	strcat(headPosFile, ".eyepos.txt");
	Setup::Headmount setupHead = Setup::Headmount(window_name_main, headPosFile);
	state = SetupPhase::SetupHeadmount;
#endif
	
	RectPair eyes;
	RectPair eyeCorners;
	cv::Mat faceROI;
	cv::Point2f headOffset;
	
	char pupilPosLogFile[1024];
	strcpy(pupilPosLogFile, argv[1]);
	strcat(pupilPosLogFile, ".pupilpos.csv");
	Detector::Pupils pupils = Detector::Pupils(pupilPosLogFile);
	
	while ( true )
	{
		cv::Mat frame_gray;
		if ( grabDownscaledGreyFrame(capture, &frame_gray) ) {
			
			// -- Get eye region
#if kCameraIsHeadmounted
			if (state == SetupHeadmount) {
				if (setupHead.waitForInput(frame_gray, &eyes, &eyeCorners)) {
					state = SetupPhase::SetupComplete;
				} else {
					imshow(window_name_main, frame_gray);
					continue; // eyes still not selected
				}
			}
			// whole image is 'face' area for eye detection
			faceROI = frame_gray;
#else
			// Apply the classifier to the frame
			cv::Rect face_r = detectFace.findFace(frame_gray);
			headOffset = face_r.tl();
			faceROI = frame_gray(face_r);
			eyes = detectFace.findEyes(faceROI);
#endif
			
			PointPair pp = pupils.find( faceROI, eyes, headOffset );
			PointPair corner = pupils.findCorners( faceROI, eyeCorners, headOffset );
			
			if (state == SetupComplete) {
				if( cv::waitKey(10) == 27 ) // esc key
					return EXIT_SUCCESS;
			}
#if kSetupEyeCoordinateSpace
			else if (state == SetupEyeCoordinateSpace) {
				if (setupECS.waitForInput(frame_gray, eyes, pp, headOffset)) {
					state = SetupPhase::SetupComplete;
				}
			}
#endif
			// draw pupil center on main image
			circle(frame_gray, pp.first, 3, 1234);
			circle(frame_gray, pp.second, 3, 1234);
			drawMarker(frame_gray, corner.first, 200);
			drawMarker(frame_gray, corner.second, 200);
			imshow(window_name_main, frame_gray);
		}
	}
	
	return EXIT_SUCCESS;
}

