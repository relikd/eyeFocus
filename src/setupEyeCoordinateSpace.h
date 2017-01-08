#ifndef setupEyeCoordinateSpace_h
#define setupEyeCoordinateSpace_h

#include "constants.h"
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

struct EyeSpaceData {
	cv::Rect2f leftEyeRegion, rightEyeRegion;
	cv::Point2f leftPupil, rightPupil;
	
	EyeSpaceData(cv::Rect2f lr, cv::Rect2f rr, cv::Point2f lp, cv::Point2f rp) : leftEyeRegion(lr), rightEyeRegion(rr), leftPupil(lp), rightPupil(rp) {};
};

namespace Setup {
	class EyeCoordinateSpace {
	public:
		std::vector<PointPair> positions;
		
		bool waitForInput(cv::Mat image, std::pair<cv::Rect2f, cv::Rect2f> region, std::pair<cv::Point2f, cv::Point2f> pupil, cv::Point2f faceOffset);
		
	private:
		void sortPositions();
	};

}

#endif /* setupEyeCoordinateSpace_h */
