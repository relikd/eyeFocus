#ifndef KalmanPoint_hpp
#define KalmanPoint_hpp

#include <opencv2/video/tracking.hpp>

class KalmanPoint {
public:
	cv::KalmanFilter filter = cv::KalmanFilter(4,2,0);
	
	KalmanPoint(double processNoise, double measurementNoise, double initialError) {
		filter.transitionMatrix = (cv::Mat_<float>(4, 4) << 1,0,1,0,   0,1,0,1,  0,0,1,0,  0,0,0,1);
		//filter.transitionMatrix = (cv::Mat_<float>(2, 2) << 1,0, 0,1);
		
		setIdentity(filter.measurementMatrix);
		setIdentity(filter.processNoiseCov, cv::Scalar::all(processNoise));
		setIdentity(filter.measurementNoiseCov, cv::Scalar::all(measurementNoise)); // error in measurement
		setIdentity(filter.errorCovPost, cv::Scalar::all(initialError));
	};
	
	inline cv::Point2f previousPoint() {
		cv::Mat mat = filter.statePre;
		return cv::Point2f(mat.at<float>(0), mat.at<float>(1));
	}
	
	// make prediction based on previous position and acceleration
	inline cv::Point2f predict() {
		cv::Mat mat = filter.predict();
		return cv::Point2f(mat.at<float>(0), mat.at<float>(1));
	}
	
	// then correct value with actual input
	inline cv::Point2f correct(cv::Point2f measurement) {
		cv::Mat estimated = filter.correct((cv::Mat_<float>(2, 1) << measurement.x, measurement.y));
		return cv::Point2f(estimated.at<float>(0), estimated.at<float>(1));
	}
	
	inline cv::Point2f smoothedPosition(cv::Point2f measurement) {
		this->predict();
		return this->correct(measurement);
	}
};

#endif /* KalmanPoint_hpp */
