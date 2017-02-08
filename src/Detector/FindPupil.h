//
//  FindPupil.h
//  eyeFocus
//
//  Created by Oleg Geier on 07/02/17.
//
//

#ifndef FindPupil_h
#define FindPupil_h

#include <opencv2/imgproc/imgproc.hpp>
#include <functional>
#include "../constants.h"
#include "../Helper/KalmanPoint.hpp"
#include "ExCuSe/algo.h"
#include "ElSe/algo.h"
#include "Timm/findEyeCenter.h"

typedef std::function<cv::RotatedRect(cv::Mat)> FindPupilFunction;
//  ---------------------------------------------------------------
// |
// |  Definition
// |
//  ---------------------------------------------------------------

class FindKalmanPupil {
	KalmanPoint KF = KalmanPoint(kKalmanProcessError, kKalmanMeasureError, kKalmanInitialError);
	
public:
	/** Returns a Kalman smoothed center point. @param offset Optional eye box offset */
	cv::RotatedRect findSmoothed(cv::Mat image, FindPupilFunction func, cv::Point2f offset = cv::Point2f(0,0)) {
		if (image.cols == 0 || image.rows == 0)
			return cv::RotatedRect();
		
		cv::RotatedRect pupil = func(image);
		
		if (pupil.center.x < 0.5 && pupil.center.y < 0.5) {
			if (kUseKalmanFilter) {
				// Reuse last point if no pupil found (eg. eyelid closed)
				pupil.center = KF.previousPoint();
			} else {
				// reset any near 0,0 value to actual 0,0 to indicate a 'not found'
				pupil.center = cv::Point2f();
			}
		} else {
			pupil.center += offset;
			if (kUseKalmanFilter) {
				pupil.center = KF.smoothedPosition( pupil.center );
			}
		}
		
		return pupil;
	}
};

//  ---------------------------------------------------------------
// |
// |  Specific implementations
// |
//  ---------------------------------------------------------------

class ExCuSe {
public:
	static cv::RotatedRect find(cv::Mat image) {
		// ExCuSe eye tracking
		cv::Mat pic_th = cv::Mat::zeros(image.rows, image.cols, CV_8U);
		cv::Mat th_edges = cv::Mat::zeros(image.rows, image.cols, CV_8U);
		return EXCUSE::run(image, &pic_th, &th_edges, true);
	};
};

class ElSe {
public:
	static cv::RotatedRect find(cv::Mat image) {
		return ELSE::run(image);
	};
};

class Timm {
public:
	static cv::RotatedRect find(cv::Mat image) {
		// Gradient based eye tracking (Timm)
		cv::RotatedRect ellipse;
		ellipse.center = Detector::EyeCenter::findEyeCenter(image);
		return ellipse;
	};
};

#endif /* FindPupil_h */
