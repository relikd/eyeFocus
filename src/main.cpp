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

cv::Point2f findCorner(cv::Mat src_gray, bool isLeftEye) {
	double avgValue = cv::sum(src_gray)[0] / (src_gray.cols * src_gray.rows);
	cv::Mat tmp;
	cv::GaussianBlur(src_gray, tmp, cv::Size(9,9), 0, 0);
	cv::threshold(tmp, tmp, avgValue, 255, cv::THRESH_BINARY);
//	cv::Canny(tmp, tmp, 0.1, 0.1);
	
	int y = tmp.rows/2;
	int x = tmp.cols/2 + (isLeftEye ? -10 : 10);
	int val = tmp.at<int>(y,x);
	while (x < tmp.cols && x >= 0) {
		(isLeftEye ? x++ : x--);
		if (tmp.at<int>(y,x) != val)
			break;
	}
	//imshow("sdfs", tmp);
	//cv::waitKey();
	//cv::circle(tmp, cv::Point(x,y), 3, 200);
	return cv::Point2f(x,y);
//
//	std::vector<std::vector<cv::Point> > contours;
//	std::vector<cv::Vec4i> hierarchy;
//	cv::findContours( tmp, contours, hierarchy, CV_RETR_CCOMP, CV_CHAIN_APPROX_SIMPLE );
//	
//	cv::Point2f corner;
//	cv::RotatedRect r;
//	for (std::vector<cv::Point> vpt : contours) {
//		if (vpt.size() > 50) {
//			r = fitEllipse(vpt);
//			
//			float piAngle = (r.angle / 180.0) * M_PI;
//			
//			if ((isLeftEye && (fabsf(r.angle - 45) < 15.0 || fabsf(r.angle - 225) < 15.0)) ||
//				(!isLeftEye && (fabsf(r.angle - 135) < 15.0 || fabsf(r.angle - 315) < 15.0)))
//			{
//				float dist = r.size.width / 2.0;
//				corner = r.center + cv::Point2f(cos(piAngle) * dist, sin(piAngle) * dist);
//			} else {
//				float dist = r.size.height / 2.0;
//				corner = r.center + cv::Point2f(cos(piAngle+M_PI_2) * dist, sin(piAngle+M_PI_2) * dist);
//			}
//			if (corner.inside(cv::Rect(0,0,tmp.cols,tmp.rows))) {
//				break;
//			}
//		}
//	}
//	printf("angle: %f, width: %f, height: %f\n", r.angle, r.size.width, r.size.height);
//	printf("%1.3f, %1.3f\n", r.center.x, r.center.y);
//	printf("%1.3f, %1.3f\n", corner.x, corner.y);
//	cv::ellipse(tmp, r, 200);
//	cv::circle(tmp, corner, 3, 200);
//	return tmp;
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
	strcat(pupilPosLogFile, ".pupilpos.txt");
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
			// find eye corner
			cv::drawMarker(frame_gray, eyeCorners.first.tl() + findCorner(faceROI(eyeCorners.first), true), 200);
			cv::drawMarker(frame_gray, eyeCorners.second.tl() + findCorner(faceROI(eyeCorners.second), false), 200);
//			findCorner(faceROI(eyeCorners.second), false);
#else
			// Apply the classifier to the frame
			cv::Rect face_r = detectFace.findFace(frame_gray);
			headOffset = face_r.tl();
			faceROI = frame_gray(face_r);
			eyes = detectFace.findEyes(faceROI);
#endif
			
			PointPair pp = pupils.find( faceROI, eyes, headOffset );
			
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
			imshow(window_name_main, frame_gray);
		}
	}
	
	return EXIT_SUCCESS;
}

