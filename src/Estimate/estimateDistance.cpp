#include "estimateDistance.hpp"
#include <cmath>

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

void Distance::readConfigFile(const char* path) {
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
			listDegrees.push_back(degree);
			listRatios.push_back(ratio);
		}
		fclose(file);
	} else {
		fputs("Error loading pre calculated angles file.\n", stderr);
		exit(EXIT_FAILURE);
	}
}

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

int Distance::estimate(EllipsePair pupil, PointPair corner, bool byDegrees) {
	float pupilCornerRatio = cv::norm(pupil.first.center - pupil.second.center) / cv::norm(corner.first - corner.second);
	float halfPupilDistanceInMM = (pupilCornerRatio * 35) / 2.0f; // Hard coded 3.5cm eye corner distance
	
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
		}
	}
	return bestMatching.distance;
}
