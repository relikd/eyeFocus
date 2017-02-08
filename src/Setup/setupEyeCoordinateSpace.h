//
//  setupEyeCoordinateSpace.h
//  eyeFocus
//
//  Created by Oleg Geier on 08/01/17.
//
//

#ifndef setupEyeCoordinateSpace_h
#define setupEyeCoordinateSpace_h

#include <opencv2/imgproc/imgproc.hpp>

typedef std::pair<cv::Point2f, cv::Point2f> PointPair;
typedef std::pair<cv::Rect2i, cv::Rect2i> RectPair;

struct EyeSpaceData {
	cv::Rect2f leftEyeRegion, rightEyeRegion;
	cv::Point2f leftPupil, rightPupil;
	
	EyeSpaceData(cv::Rect2f lr, cv::Rect2f rr, cv::Point2f lp, cv::Point2f rp) : leftEyeRegion(lr), rightEyeRegion(rr), leftPupil(lp), rightPupil(rp) {};
};

namespace Setup {
	class EyeCoordinateSpace {
	public:
		std::vector<PointPair> positions;
		
		bool waitForInput(cv::Mat image, RectPair region, PointPair pupil, cv::Point2i faceOffset);
		
	private:
		void sortPositions();
	};

}

#endif /* setupEyeCoordinateSpace_h */
