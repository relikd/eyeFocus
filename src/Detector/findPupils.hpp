#ifndef findPupils_hpp
#define findPupils_hpp

#include <opencv2/imgproc/imgproc.hpp>
#include "../constants.h"
#include "../Helper/KalmanPoint.hpp"
#include "findEyeCenter.h"
#include "findEyeCorner.h"

#define USE_EXCUSE_EYETRACKING 1

namespace Detector {
	class Pupils {
		FILE* file = NULL;
		KalmanPoint KFL = KalmanPoint(kKalmanProcessError, kKalmanMeasureError, kKalmanInitialError); // left pupil
		KalmanPoint KFR = KalmanPoint(kKalmanProcessError, kKalmanMeasureError, kKalmanInitialError); // right pupil
#if !USE_EXCUSE_EYETRACKING
		EyeCenter detectCenter = EyeCenter();
#endif
		EyeCorner detectCorner = EyeCorner();
		
	public:
		Pupils(const char* path = NULL);
		
		~Pupils() {
			if (file) fclose(file);
		}
		
		PointPair find( cv::Mat faceROI, RectPair eyes, cv::Point2i offset );
		cv::Point2f findSingle( cv::Mat faceROI );
		PointPair findCorners( cv::Mat faceROI, RectPair cornerRegion, cv::Point2i offset );
		
	private:
		cv::Point2f findPupil( cv::Mat faceImage, cv::Rect2i eyeRegion, bool isLeftEye );
	};
}

#endif /* findPupils_hpp */
