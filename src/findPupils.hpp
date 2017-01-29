#ifndef findPupils_hpp
#define findPupils_hpp

#include <opencv2/imgproc/imgproc.hpp>
#include "constants.h"
#include "KalmanPoint.hpp"
#include "findEyeCenter.h"
#include "findEyeCorner.h"

namespace Detector {
	class Pupils {
		FILE* file = NULL;
		KalmanPoint KFL = KalmanPoint(kKalmanProcessError, kKalmanMeasureError, kKalmanInitialError); // left pupil
		KalmanPoint KFR = KalmanPoint(kKalmanProcessError, kKalmanMeasureError, kKalmanInitialError); // right pupil
		EyeCenter detectCenter = EyeCenter();
		EyeCorner detectCorner = EyeCorner();
		
	public:
		Pupils(const char* path = NULL);
		
		~Pupils() {
			if (file) fclose(file);
		}
		
		PointPair find( cv::Mat faceROI, RectPair eyes, cv::Point2f offset );
		PointPair findCorners( cv::Mat faceROI, RectPair cornerRegion, cv::Point2f offset );
		
	private:
		cv::Point2f findPupil( cv::Mat &faceImage, cv::Rect2f &eyeRegion, bool isLeftEye );
	};
}

#endif /* findPupils_hpp */
