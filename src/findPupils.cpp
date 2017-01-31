#include "findPupils.hpp"
#include "Debug.h"

using namespace Detector;

static Debug debugEye(EyeImage);

Pupils::Pupils(const char* path) {
	if (path) {
#ifdef _WIN32
		fopen_s(&file, path, "w");
#else
		file = fopen(path, "w");
#endif
		fprintf(file, "pLx,pLy,pRx,pRy,PupilDistance,cLx,cLy,cRx,cRy,CornerDistance\n");
	}
}

void printDebugOutput(FILE* f, cv::Point2f a, cv::Point2f b, bool isCorner) {
	float dist = cv::norm(a - b);
	if (isCorner) printf(" | ");
	printf("%s: ([%1.1f,%1.1f],[%1.1f,%1.1f], dist: %1.1f)", (isCorner?"corner":"pupil"), a.x, a.y, b.x, b.y, dist);
	if (isCorner) printf("\n");
	if (f) fprintf(f, "%1.1f,%1.1f,%1.1f,%1.1f,%1.2f%c", a.x, a.y, b.x, b.y, dist, (isCorner?'\n':','));
}

PointPair Pupils::findCorners( cv::Mat faceROI, RectPair cornerRegion, cv::Point2f offset ) {
	// find eye corner
	cv::Point2f leftCorner  = detectCorner.findByAvgColor(faceROI(cornerRegion.first), true);
	cv::Point2f rightCorner = detectCorner.findByAvgColor(faceROI(cornerRegion.second), false);
	leftCorner  += offset + cornerRegion.first.tl();
	rightCorner += offset + cornerRegion.second.tl();
	
	printDebugOutput(file, leftCorner, rightCorner, true);
	
	return std::make_pair(leftCorner, rightCorner);
}

PointPair Pupils::find( cv::Mat faceROI, RectPair eyes, cv::Point2f offset ) {
#if DEBUG_PLOT_ENABLED
	debugEye.setImage(faceROI);
#endif
	
	//-- Find Eye Centers
	cv::Point2f leftPupil = findPupil( faceROI, eyes.first, true ) + offset;
	cv::Point2f rightPupil = findPupil( faceROI, eyes.second, false ) + offset;
	
	printDebugOutput(file, leftPupil, rightPupil, false);
	
	//cv::Rect roi( cv::Point( 0, 0 ), faceROI.size());
	//cv::Mat destinationROI = debugFace( roi );
	//faceROI.copyTo( destinationROI );
	
#if DEBUG_PLOT_ENABLED
	debugEye.display(window_name_face);
#endif
	
	return std::make_pair(leftPupil, rightPupil);
}

cv::Point2f Pupils::findPupil( cv::Mat &faceImage, cv::Rect2f &eyeRegion, bool isLeftEye )
{
	if (eyeRegion.area()) {
		cv::Point2f pupil = detectCenter.findEyeCenter(faceImage, eyeRegion, (isLeftEye ? window_name_left_eye : window_name_right_eye) );
		
		if (pupil.x < 0.5 && pupil.y < 0.5) {
			if (kUseKalmanFilter) {
				// Reuse last point if no pupil found (eg. eyelid closed)
				pupil = (isLeftEye ? KFL : KFR).previousPoint();
			} else {
				// reset any near 0,0 value to actual 0,0 to indicate a 'not found'
				pupil = cv::Point2f();
			}
		}
		
		if (kUseKalmanFilter) {
			pupil = (isLeftEye ? KFL : KFR).smoothedPosition( pupil );
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
		debugEye.addCircle(pupil + eyeRegion.tl());
#endif
		
		return pupil + eyeRegion.tl(); // add offset
	}
	return cv::Point2f();
}
