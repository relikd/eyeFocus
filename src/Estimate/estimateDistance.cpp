//
//  estimateDistance.cpp
//  eyeFocus
//
//  Created by Oleg Geier on 03/02/17.
//
//

#include "estimateDistance.h"
#include <cmath>
#include "../Helper/QR.h"

using namespace Estimate;



void Distance::initialize(std::vector<float> pupilDistance, std::vector<int> focusDistance) {
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
	
	initialize(count, pplDist, fcsDist);
	
	delete [] pplDist;
	delete [] fcsDist;
}

void Distance::initialize(int count, float* pupilDistance, int* focusDistance) {
	int equations = count;
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
	
	
	double* A = new double[unknowns * equations];
	double* b = new double[equations];
	
	for (int i = 0; i < count; i++) {
		for (int a = 0; a < unknowns; a++) {
			A[i * unknowns + a] = pow(pupilDistance[i], e(a));
		}
		b[i] = focusDistance[i];
		printf("%d mm => %1.1f px\n", focusDistance[i], pupilDistance[i]);
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

void Distance::printEquation(bool newline) {
	if (newline)
		printf("\nf(x) = ");
	for (int i = 0; i < unknowns; i++) {
		if (_x[i] >= 0 && i != 0)
			printf("+");
		printf("%f", _x[i]);
		if (e(i) > 0)
			printf("x");
		if (e(i) > 1)
			printf("^%d", e(i));
	}
	if (newline)
		printf("\n");
}

void Distance::drawOnFrame(cv::Mat &frame, double distance) {
	char strEst[6];
	snprintf(strEst, 6*sizeof(char), "%dcm", cvRound(distance/10));
	cv::putText(frame, strEst, cv::Point(frame.cols - 220, frame.rows - 10), cv::FONT_HERSHEY_PLAIN, 5.0f, cv::Scalar(255,255,255));
}
