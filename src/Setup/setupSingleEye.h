//
//  setupSingleEye.h
//  eyeFocus
//
//  Created by Oleg Geier on 03/02/17.
//
//

#ifndef setupSingleEye_h
#define setupSingleEye_h

#include <opencv2/imgproc/imgproc.hpp>
#include "../Helper/FrameReader.h"
#include "../Detector/FindPupil.h"

namespace Setup {
	class SingleEye {
	public:
		cv::Point2f cm20 = cv::Point2f(0,0);
		cv::Point2f cm50 = cv::Point2f(0,0);
		cv::Point2f cm80 = cv::Point2f(0,0);
		
		SingleEye(FrameReader &fr, FindKalmanPupil* tracker);
	};
}

#endif /* setupSingleEye_h */
