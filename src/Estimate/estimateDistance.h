//
//  estimateDistance.h
//  eyeFocus
//
//  Created by Oleg Geier on 03/02/17.
//
//

#ifndef estimateDistance_hpp
#define estimateDistance_hpp

#include <opencv2/imgproc/imgproc.hpp>
#include <stdio.h>
#include <vector>

namespace Estimate {
	struct FocalLevel {
		int distance = 0;
		float min, avg, max;
	};
	
	
	class Distance {
		FILE* file = NULL;
		std::vector<FocalLevel> listDegrees;
		std::vector<FocalLevel> listRatios;
		
	public:
		/** @param path Path to distance level configuration file */
		Distance(const char* path);
		
		int estimate(cv::RotatedRect leftPupil, cv::RotatedRect rightPupil, cv::Point2f leftCorner, cv::Point2f rightCorner, bool byDegrees = true); // otherwise by ratio
		
		static int singlePupilHorizontal(float x, float cm20, float cm50, float cm80);
	};
}

#endif /* estimateDistance_hpp */
