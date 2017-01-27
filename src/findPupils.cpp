#include "findPupils.hpp"
#include "Debug.h"

using namespace Detector;

static Debug debugEye(EyeImage);

PointPair Pupils::find( cv::Mat faceROI, RectPair eyes, cv::Point2f offset ) {
	debugEye.setImage(faceROI);
	
	//-- Find Eye Centers
	cv::Point2f leftPupil = findPupil( faceROI, eyes.first, true ) + offset;
	cv::Point2f rightPupil = findPupil( faceROI, eyes.second, false ) + offset;
	
	float eyeDistance = cv::norm(leftPupil - rightPupil);
	printf("L[%1.1f,%1.1f] - R[%1.1f,%1.1f] (distance: %1.1f)\n",
		   leftPupil.x, leftPupil.y,
		   rightPupil.x, rightPupil.y, eyeDistance);
	
	//cv::Rect roi( cv::Point( 0, 0 ), faceROI.size());
	//cv::Mat destinationROI = debugFace( roi );
	//faceROI.copyTo( destinationROI );
	
	debugEye.display(window_name_face);
	
	return std::make_pair(leftPupil, rightPupil);
}

cv::Point2f Pupils::findPupil( cv::Mat &faceImage, cv::Rect2f &eyeRegion, bool isLeftEye )
{
	if (eyeRegion.area()) {
		cv::Point2f pupil = detectCenter.findEyeCenter(faceImage, eyeRegion, (isLeftEye ? window_name_left_eye : window_name_right_eye) );
		
		if (pupil.x < 0.5 && pupil.y < 0.5) {
			if (kUseKalmanFilter) {
				// Reuse last point if no pupil found (eg. eyelid closed)
				cv::Mat prevPos = (isLeftEye ? KFL : KFR).statePre;
				pupil = cv::Point2f(prevPos.at<float>(0), prevPos.at<float>(1));
			} else {
				// reset any near 0,0 value to actual 0,0 to indicate a 'not found'
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
