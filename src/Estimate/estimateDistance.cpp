//
//  estimateDistance.cpp
//  eyeFocus
//
//  Created by Oleg Geier on 03/02/17.
//
//

#include "estimateDistance.h"
#include <cmath>
#include <fstream>
#include "../Helper/QR.h"
#include "../Helper/Graph.h"

using namespace Estimate;

void Distance::initialize(std::vector<float> pupilDistance, std::vector<int> focusDistance, int maxExponent) {
	if (pupilDistance.size() != focusDistance.size() || pupilDistance.size() == 0) {
		fputs("Distance Estimation vector size mismatch. pupilDistance and focusDistance must have same size().\n", stderr);
		return;
	}
	
	int count = pupilDistance.size();
	
	float* pplDist = new float[count];
	int* fcsDist = new int[count];
	for (int i = 0; i < count; i++) {
		pplDist[i] = pupilDistance[i];
		fcsDist[i] = focusDistance[i];
	}
	
	initialize(count, pplDist, fcsDist, maxExponent);
	
	delete [] pplDist;
	delete [] fcsDist;
}

void Distance::initialize(int count, float* pupilDistance, int* focusDistance, int maxExponent) {
	int equations = count;
	
	if (maxExponent > 0) {
		unknowns = maxExponent;
	} else {
		unknowns = 0;
		int* uniqueFocusDist = new int[count];
		for (int i = 0; i < count; i++) {
			bool duplicate = false;
			for (int u = 0; u < unknowns; u++) {
				if (uniqueFocusDist[u] == focusDistance[i]) {
					duplicate = true;
					break;
				}
			}
			if (!duplicate) {
				uniqueFocusDist[unknowns] = focusDistance[i];
				++unknowns;
			}
		}
		delete [] uniqueFocusDist;
	}
	
	
	
	double* A = new double[unknowns * equations];
	double* b = new double[equations];
	
	for (int i = 0; i < count; i++) {
		for (int a = 0; a < unknowns; a++) {
			A[i * unknowns + a] = pow(pupilDistance[i], e(a));
		}
		b[i] = focusDistance[i];
		printf("%d mm => %1.2f px\n", focusDistance[i], pupilDistance[i]);
	}
	
	delete [] _x;
	_x = QR::solve(equations, unknowns, A, b);
	delete [] A;
	delete [] b;
}

double Distance::estimate(float pplDist) {
	double sum = 0;
	for (int i = 0; i < unknowns; i++)
		sum += _x[i] * pow(pplDist, e(i));
	return sum;
}

inline int Distance::e(int index) {
	return unknowns - index - 1; // Ax^2 + Bx + C  // (without -1: Ax^3 + Bx^2 + Cx )
}


//  ---------------------------------------------------------------
// |
// |  Printing and Screen Drawing
// |
//  ---------------------------------------------------------------

void Distance::drawOnFrame(cv::Mat &frame, double distance) {
	char strEst[12];
	snprintf(strEst, 12*sizeof(char), "%dcm", cvRound(distance/10));
	cv::Size s = cv::getTextSize(strEst, cv::FONT_HERSHEY_PLAIN, 5.0f, 1, NULL);
	cv::putText(frame, strEst, cv::Point(frame.cols - s.width, frame.rows - 10), cv::FONT_HERSHEY_PLAIN, 5.0f, cv::Scalar(255,255,255));
}

void Distance::printEquation(bool newline) {
	if (newline)
		printf("\nf(x) = ");
	for (int i = 0; i < unknowns; i++) {
		if (_x[i] >= 0 && i != 0)
			printf("+");
		printf("%.42e", _x[i]);
		if (e(i) > 0)
			printf("x");
		if (e(i) > 1)
			printf("^%d", e(i));
	}
	if (newline)
		printf("\n");
}

cv::Mat Distance::graphFunction(int min_x, int max_x, std::vector<cv::Point2f> measurement) {
	Graph plot = Graph(cv::Point2i(min_x, 0), cv::Point2i(max_x, 1050), cv::Size(640,480));
	plot.addAxis(cv::Point2i(5, 100), cv::Point2i(1, 25), cv::Point2f(1, 1e-1));
	plot.addFunction(0.1f, [this](float &x){
		return estimate(x);
	});
	plot.addMarkers(measurement);
	plot.addAxisLabels("px", "cm");
	return plot.img;
}

cv::Mat Distance::graphUncertainty(int min_x, int max_x, const char* path) {
	std::ofstream outputStream;
	if (path) outputStream = std::ofstream(path);
	
	Graph plot = Graph(cv::Point2i(0, 0), cv::Point2i(1050, 65));
	plot.addAxis(cv::Point2i(100, 10), cv::Point2i(25, 5), cv::Point2f(1e-1, 1));
	plot.addFunction(0.1, [this,&outputStream](float &x){
		double thisEst = estimate(x);
		double diff = fabs(thisEst - estimate(x+1));
		x = thisEst;
		outputStream << x << "\t" << diff << "\n";
		return diff;
	}, min_x, max_x);
	plot.addAxisLabels("cm", "mm");
	outputStream.close();
	return plot.img;
}

