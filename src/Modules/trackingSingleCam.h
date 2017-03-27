//
//  trackingSingleCam.h
//  eyeFocus
//
//  Created by Oleg Geier on 27/03/17.
//
//

#ifndef trackingSingleCam_h
#define trackingSingleCam_h

#include <opencv2/imgproc/imgproc.hpp>
#include "../Helper/FrameReader.h"
#include "../Detector/findPupil.h"

//  ---------------------------------------------------------------
// |
// |  Single Eye Tracking Mode (1 Camera, 1 Pupil)
// |
//  ---------------------------------------------------------------

namespace Tracking {
	class SingleCam {
		cv::Point2f cm20 = cv::Point2f(0,0);
		cv::Point2f cm50 = cv::Point2f(0,0);
		cv::Point2f cm80 = cv::Point2f(0,0);
		
	public:
		/** Single pupil is filling the complete cam image */
		void start(FrameReader &fr);
		void init(FrameReader &fr, FindKalmanPupil* tracker);
	};
}

#endif /* trackingSingleCam_h */
