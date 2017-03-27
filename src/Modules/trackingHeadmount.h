//
//  trackingHeadmount.h
//  eyeFocus
//
//  Created by Oleg Geier on 27/03/17.
//
//

#ifndef trackingHeadmount_h
#define trackingHeadmount_h

#include <opencv2/imgproc/imgproc.hpp>
#include "../Helper/FrameReader.h"
#include "../Detector/findFace.h"

//  ---------------------------------------------------------------
// |
// |  Tracking with 1 Camera. Both pupils are visible. [optional: Face Detection]
// |
//  ---------------------------------------------------------------

namespace Tracking {
	class Headmount {
		Detector::Face faceDetector;
		
		cv::Rect2i eyeBox[2];
		cv::Rect2i eyeCorner[2];
		
	public:
		void start(FrameReader &fr, bool findFace = false);
		
	private:
		void manuallySelectEyeRegion(FrameReader &fr);
		void drawDebugPlot(cv::Mat &frame, cv::Rect2i box, cv::RotatedRect pupil);

	};
}

#endif /* trackingHeadmount_h */
