//
//  trackingDualCam.h
//  eyeFocus
//
//  Created by Oleg Geier on 12/02/17.
//
//

#ifndef trackingDualCam_h
#define trackingDualCam_h

#include <opencv2/imgproc/imgproc.hpp>
#include "../Estimate/estimateDistance.h"

namespace Tracking {
	class DualCam {
		std::vector<float> pplDistancePoints;
		std::vector<int> focalPoints;
		int currentFocusDistance = 200;
		
		Estimate::Distance distEst;
		bool setupFinished = false;
		
	public:
		DualCam(const char *path, const char* file);
		
	private:
		void setupPhase(cv::Mat &frame, cv::Point2f pLeft, cv::Point2f pRight);
		// Calibration
		void loadCalibrationFile(const char* path);
		void saveCalibrationFile(const char* path);
		inline void clearMeasurement();
		inline void finalizeSetup();
		// Graph / Plot
		inline void drawPlot(int winx, cv::String window, cv::Mat plot, cv::String filename = "");
		void showGraph(Estimate::Distance &estimator);
	};
}

#endif /* trackingDualCam_h */
