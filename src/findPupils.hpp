#ifndef findPupils_hpp
#define findPupils_hpp

#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/video/tracking.hpp>
#include "constants.h"
#include "findEyeCenter.h"
#include "findEyeCorner.h"

namespace Detector {
	class Pupils {
		cv::KalmanFilter KFL = cv::KalmanFilter(4,2,0); // left pupil
		cv::KalmanFilter KFR = cv::KalmanFilter(4,2,0); // right pupil
		EyeCenter detectCenter = EyeCenter();
		EyeCorner detectCorner = EyeCorner();
		
	public:
		Pupils() {
			if (kUseKalmanFilter) { // Init Kalman filter
				KFL.transitionMatrix = (cv::Mat_<float>(4, 4) << 1,0,1,0,   0,1,0,1,  0,0,1,0,  0,0,0,1);
				KFR.transitionMatrix = (cv::Mat_<float>(4, 4) << 1,0,1,0,   0,1,0,1,  0,0,1,0,  0,0,0,1);
				//KFL.transitionMatrix = (cv::Mat_<float>(2, 2) << 1,0, 0,1);
				//KFR.transitionMatrix = (cv::Mat_<float>(2, 2) << 1,0, 0,1);
				
				setIdentity(KFL.measurementMatrix);
				setIdentity(KFL.processNoiseCov, cv::Scalar::all(kKalmanProcessError));
				setIdentity(KFL.measurementNoiseCov, cv::Scalar::all(kKalmanMeasureError)); // error in measurement
				setIdentity(KFL.errorCovPost, cv::Scalar::all(kKalmanInitialError));
				setIdentity(KFR.measurementMatrix);
				setIdentity(KFR.processNoiseCov, cv::Scalar::all(kKalmanProcessError));
				setIdentity(KFR.measurementNoiseCov, cv::Scalar::all(kKalmanMeasureError));
				setIdentity(KFR.errorCovPost, cv::Scalar::all(kKalmanInitialError));
			}
		};
		
		PointPair find( cv::Mat faceROI, RectPair eyes, cv::Point2f offset );
		
	private:
		cv::Point2f findPupil( cv::Mat &faceImage, cv::Rect2f &eyeRegion, bool isLeftEye );
	};
}

#endif /* findPupils_hpp */
