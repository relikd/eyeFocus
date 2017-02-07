#include "findPupils.hpp"
#include "../Helper/Debug.h"
#include "ExCuSe/algo.h"
#include "ElSe/algo.h"

using namespace Detector;

static Debug debugEye(EyeImage);

Pupils::Pupils(const char* path) {
	if (path) {
#ifdef _WIN32
		fopen_s(&file, path, "w");
#else
		file = fopen(path, "w");
#endif
		if (file) fprintf(file, "pLx,pLy,pRx,pRy,PupilDistance,cLx,cLy,cRx,cRy,CornerDistance\n");
	}
}

inline void addIntOffset(cv::Point2f &base, cv::Point2i offset) {
	base.x += offset.x;
	base.y += offset.y;
}

void printDebugOutput(FILE* f, cv::Point2f a, cv::Point2f b, bool isCorner) {
	double dist = cv::norm(a - b);
	if (isCorner) printf(" | ");
	printf("%s: ([%1.1f,%1.1f],[%1.1f,%1.1f], dist: %1.1f)", (isCorner?"corner":"pupil"), a.x, a.y, b.x, b.y, dist);
	if (isCorner) printf("\n");
	if (f) fprintf(f, "%1.1f,%1.1f,%1.1f,%1.1f,%1.2f%c", a.x, a.y, b.x, b.y, dist, (isCorner?'\n':','));
}

PointPair Pupils::findCorners( cv::Mat faceROI, RectPair cornerRegion, cv::Point2i offset ) {
	// find eye corner
	cv::Point2f leftCorner  = detectCorner.findByAvgColor(faceROI(cornerRegion.first), true);
	cv::Point2f rightCorner = detectCorner.findByAvgColor(faceROI(cornerRegion.second), false);
	addIntOffset(leftCorner, offset + cornerRegion.first.tl());
	addIntOffset(rightCorner, offset + cornerRegion.second.tl());
	
	printDebugOutput(file, leftCorner, rightCorner, true);
	
	return std::make_pair(leftCorner, rightCorner);
}

cv::RotatedRect Pupils::findSingle( cv::Mat faceROI ) {
	cv::Rect2i rectWithoutEdge(faceROI.cols*0.1, faceROI.rows*0.1, faceROI.cols*0.8, faceROI.rows*0.8);
	return findPupil( faceROI, rectWithoutEdge, true );
}

EllipsePair Pupils::find( cv::Mat faceROI, RectPair eyes, cv::Point2i offset ) {
#if DEBUG_PLOT_ENABLED
	debugEye.setImage(faceROI);
#endif
	
	//-- Find Eye Centers
	cv::RotatedRect leftPupil = findPupil( faceROI, eyes.first, true );
	cv::RotatedRect rightPupil = findPupil( faceROI, eyes.second, false );
	addIntOffset(leftPupil.center, offset);
	addIntOffset(rightPupil.center, offset);

	printDebugOutput(file, leftPupil.center, rightPupil.center, false);
	
	//cv::Rect roi( cv::Point( 0, 0 ), faceROI.size());
	//cv::Mat destinationROI = debugFace( roi );
	//faceROI.copyTo( destinationROI );
	
#if DEBUG_PLOT_ENABLED
	debugEye.display(window_name_face);
#endif
	
	return std::make_pair(leftPupil, rightPupil);
}

cv::RotatedRect Pupils::findPupil( cv::Mat faceImage, cv::Rect2i eyeRegion, bool isLeftEye )
{
	if (eyeRegion.area()) {
#if USE_EXCUSE_EYETRACKING
		// ExCuSe eye tracking
		cv::Mat sub = faceImage(eyeRegion);
//		cv::resize(sub, sub, cv::Size(sub.cols/2, sub.rows/2));
		cv::Mat pic_th = cv::Mat::zeros(sub.rows, sub.cols, CV_8U);
		cv::Mat th_edges = cv::Mat::zeros(sub.rows, sub.cols, CV_8U);
//		cv::RotatedRect pupil = run(sub, &pic_th, &th_edges, true);
//		pupil.center.x *= 2;
//		pupil.center.y *= 2;
//		pupil.size.width *= 2;
//		pupil.size.height *= 2;
		cv::RotatedRect pupil = ELSE::run(faceImage(eyeRegion));
#else
		// Gradient based eye tracking (Timm)
		cv::RotatedRect pupil;
		pupil.center = detectCenter.findEyeCenter(faceImage, eyeRegion, (isLeftEye ? window_name_left_eye : window_name_right_eye) );
#endif
		
//		cv::Mat empty = cv::Mat::zeros(faceImage.rows, faceImage.cols, CV_8U);
//		addIntOffset(elipse.center, eyeRegion.tl());
//		ellipse(empty, elipse, 123);
//		imshow("tmp", empty);
		
		if (pupil.center.x < 0.5 && pupil.center.y < 0.5) {
			if (kUseKalmanFilter) {
				// Reuse last point if no pupil found (eg. eyelid closed)
				pupil.center = (isLeftEye ? KFL : KFR).previousPoint();
			} else {
				// reset any near 0,0 value to actual 0,0 to indicate a 'not found'
				pupil.center = cv::Point2f();
			}
		}
		
		if (kUseKalmanFilter) {
			pupil.center = (isLeftEye ? KFL : KFR).smoothedPosition( pupil.center );
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
		cv::Rect2f leftRegion(eyeRegion.x, eyeRegion.y, pupil.center.x, eyeRegion.height / 2);
		leftRegion.y += leftRegion.height / 2;
		
		cv::Rect2f rightRegion(leftRegion);
		rightRegion.x += pupil.center.x;
		rightRegion.width = eyeRegion.width - pupil.center.x;
		
		if (kEnableEyeCorner) {
			cv::Point2f leftCorner = detectCorner.find(faceImage(leftRegion), isLeftEye, false);
			cv::Point2f rightCorner = detectCorner.find(faceImage(rightRegion), isLeftEye, true);
			debugEye.addCircle( leftCorner + leftRegion.tl() , 200 );
			debugEye.addCircle( rightCorner + rightRegion.tl() , 200 );
		}
		
		// draw eye region
		debugEye.addRectangle(eyeRegion);
		
		// draw tiled eye box
		debugEye.addRectangle(leftRegion, 200);
		debugEye.addRectangle(rightRegion, 200);
		
		// draw eye center
		addIntOffset(pupil.center, eyeRegion.tl());
		debugEye.addCircle(pupil.center);
#else
		addIntOffset(pupil.center, eyeRegion.tl());
#endif
		return pupil;
	}
	return cv::RotatedRect();
}
