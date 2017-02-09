//
//  estimateDistance.cpp
//  eyeFocus
//
//  Created by Oleg Geier on 03/02/17.
//
//

#include "estimateDistance.h"
#include <cmath>
#include "../constants.h"

using namespace Estimate;

bool readLine(FILE* f, FocalLevel *degrees, FocalLevel *ratio) {
	int dist = 0;
	if (fscanf(f, "%d;%f;%f;%f;%f;%f;%f\n", &dist, &degrees->min, &degrees->avg, &degrees->max, &ratio->min, &ratio->avg, &ratio->max) != EOF) {
		degrees->distance = dist;
		ratio->distance = dist;
		return true;
	}
	return false;
}

Distance::Distance(const char* path) {
	if (path == NULL)
		return;
	
#ifdef _WIN32
	fopen_s(&file, path, "r");
#else
	file = fopen(path, "r");
#endif
	
	if (file) {
		fscanf(file, "[Config]\n");
		FocalLevel degree;
		FocalLevel ratio;
		while (readLine(file, &degree, &ratio)) {
			if (degree.distance <= 50) {
				listDegrees.push_back(degree);
				listRatios.push_back(ratio);
			}
		}
		fclose(file);
	} else {
		fputs("Error loading pre calculated angles file.\n", stderr);
//		exit(EXIT_FAILURE);
	}
}

int Distance::estimate(cv::RotatedRect leftPupil, cv::RotatedRect rightPupil, cv::Point2f leftCorner, cv::Point2f rightCorner, bool byDegrees)
{
	float pupilCornerRatio = cv::norm(leftPupil.center - rightPupil.center) / cv::norm(leftCorner - rightCorner);
	float halfPupilDistanceInMM = (pupilCornerRatio * kEyeCornerDistanceInMM) / 2.0f;
	
	FocalLevel bestMatching;
	float bestError = 999;
	
	for (FocalLevel &fl : (byDegrees ? listDegrees : listRatios) ) {
		float angle;
		if (byDegrees)
			angle = 2 * atanf( halfPupilDistanceInMM / (fl.distance * 10) ) * 180 / M_PI;
		else
			angle = pupilCornerRatio;
		
		float range = fl.max - fl.min;
		float curError = std::fabs(angle - fl.avg);
		if (byDegrees) curError /= range;
		else           curError *= range;
		
		if (bestError > curError) {
			bestError = curError;
			bestMatching = fl;
			//printf("set: ");
		}
		//printf("%d %1.3f\n", fl.distance, curError);
	}
	return bestMatching.distance;
}

// static
int Distance::singlePupilHorizontal(float x, float cm20, float cm50, float cm80) {
	double lowerRange = std::pow(cm50 / cm20, 1/30.0);
	double upperRange = std::pow(cm80 / cm50, 1/30.0);
	
	float initLower = cm20 * std::pow(lowerRange, -10); // start at 10cm
	float initUpper = cm50;
	
	int bestMatchLower = 0;
	int bestMatchUpper = 0;
	
	float diff = 999;
	do {
		float newDiff = std::fabs(initLower - x);
		if (newDiff > diff) {
			return 10 + bestMatchLower - 1;
		}
		diff = newDiff;
		initLower *= lowerRange;
	} while (++bestMatchLower < 40);
	
	diff = 999;
	
	do {
		float newDiff = std::fabs(initUpper - x);
		if (newDiff > diff) {
			return 50 + bestMatchUpper - 1;
		}
		diff = newDiff;
		initUpper *= upperRange;
	} while (++bestMatchUpper < 30);
	
	return 100;
}
