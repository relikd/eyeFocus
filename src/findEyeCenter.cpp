#include "findEyeCenter.h"

#include <opencv2/highgui/highgui.hpp>
#include <queue>

#include "constants.h"


using namespace Detector;

// Pre-declarations
cv::Mat floodKillEdges(cv::Mat &mat);

#pragma mark Helpers

cv::Point unscalePoint(cv::Point p, cv::Rect origSize) {
	float ratio = (((float)kFastEyeWidth)/origSize.width);
	int x = round(p.x / ratio);
	int y = round(p.y / ratio);
	return cv::Point(x,y);
}

void scaleToFastSize(const cv::Mat &src,cv::Mat &dst) {
	cv::resize(src, dst, cv::Size(kFastEyeWidth,(((float)kFastEyeWidth)/src.cols) * src.rows));
}

double computeDynamicThreshold(const cv::Mat &mat, double stdDevFactor) {
	cv::Scalar stdMagnGrad, meanMagnGrad;
	cv::meanStdDev(mat, meanMagnGrad, stdMagnGrad);
	double stdDev = stdMagnGrad[0] / sqrt(mat.rows*mat.cols);
	return stdDevFactor * stdDev + meanMagnGrad[0];
}


#pragma mark Main Algorithm

void testPossibleCentersFormula(int x, int y, const cv::Mat &weight, double gx, double gy, cv::Mat &out) {
	out.forEach<double>([&](double &Or, const int position[]) {
		const int cy = position[0];
		const int cx = position[1];
		if (x == cx && y == cy) {
			return;
		}
		double dx = x - cx;
		double dy = y - cy;
		// normalize d
		double magnitude = sqrt((dx * dx) + (dy * dy));
		dx = dx / magnitude;
		dy = dy / magnitude;
		double dotProduct = dx*gx + dy*gy;
		dotProduct = std::max(0.0,dotProduct);
		// square and multiply by the weight
		if (kEnableWeight) {
			const unsigned char *Wr = weight.ptr<unsigned char>(cy);
			Or += dotProduct * dotProduct * (Wr[cx]/kWeightDivisor);
		} else {
			Or += dotProduct * dotProduct;
		}
	});
}

cv::Point EyeCenter::findEyeCenter(cv::Mat face, cv::Rect eye, std::string debugWindow) {
	cv::Mat eyeROIUnscaled = face(eye);
	cv::Mat eyeROI;
	scaleToFastSize(eyeROIUnscaled, eyeROI);
	
	//-- Find the gradient
	cv::Mat gradientX, gradientY;
	cv::Sobel( eyeROI, gradientX, CV_64F, 1, 0, 1, 0.5 ); // 1.1x slower, but straigth forward understanding
	cv::Sobel( eyeROI, gradientY, CV_64F, 0, 1, 1, 0.5 );
	
	//-- Normalize and threshold the gradient
	// compute all the magnitudes
	cv::Mat mags;
	cv::magnitude(gradientX, gradientY, mags); // 3.5x faster
	//compute the threshold
	double gradientThresh = computeDynamicThreshold(mags, kGradientThreshold);
//	double gradientThresh = kGradientThreshold;
//	double gradientThresh = 0;
	//normalize
	for (int y = 0; y < eyeROI.rows; ++y) {
		double *Xr = gradientX.ptr<double>(y), *Yr = gradientY.ptr<double>(y);
		const double *Mr = mags.ptr<double>(y);
		for (int x = 0; x < eyeROI.cols; ++x) {
			double gX = Xr[x], gY = Yr[x];
			double magnitude = Mr[x];
			if (magnitude > gradientThresh) {
				Xr[x] = gX/magnitude;
				Yr[x] = gY/magnitude;
			} else {
				Xr[x] = 0.0;
				Yr[x] = 0.0;
			}
		}
	}
	imshow(debugWindow, gradientX);
	
	//-- Create a blurred and inverted image for weighting
	cv::Mat weight;
	GaussianBlur( eyeROI, weight, cv::Size( kWeightBlurSize, kWeightBlurSize ), 0, 0 );
	cv::bitwise_not(weight, weight); // 20x faster
	
//	imshow(debugWindow,weight);
	//-- Run the algorithm!
	cv::Mat outSum = cv::Mat::zeros(eyeROI.rows,eyeROI.cols,CV_64F);
	// for each possible gradient location
	// Note: these loops are reversed from the way the paper does them
	// it evaluates every possible center for each gradient location instead of
	// every possible gradient location for every center.
	printf("Eye Size: %ix%i\n",outSum.cols,outSum.rows);
	gradientX.forEach<double>([&gradientY, &weight, &outSum](const double &gX, const int position[]) {
		const int y = position[0];
		const int x = position[1];
		const double gY = gradientY.ptr<double>(y)[x];
		if (gX == 0.0 && gY == 0.0) {
			return;
		}
		testPossibleCentersFormula(x, y, weight, gX, gY, outSum);
	}); // 1.6x faster
	
	// scale all the values down, basically averaging them
	double numGradients = (weight.rows*weight.cols);
	cv::Mat out;
	outSum.convertTo(out, CV_32F,1.0/numGradients);
//	imshow(debugWindow,out);
	//-- Find the maximum point
	cv::Point maxP;
	double maxVal;
	cv::minMaxLoc(out, NULL,&maxVal,NULL,&maxP);
	//-- Flood fill the edges
	if(kEnablePostProcess) {
		cv::Mat floodClone;
//		double floodThresh = computeDynamicThreshold(out, 1.5);
		double floodThresh = maxVal * kPostProcessThreshold;
		cv::threshold(out, floodClone, floodThresh, 0.0f, cv::THRESH_TOZERO);
		
		cv::Mat mask = floodKillEdges(floodClone);
//		imshow(debugWindow + " Mask",mask);
//		imshow(debugWindow,out);
		// redo max
		cv::minMaxLoc(out, NULL,&maxVal,NULL,&maxP,mask);
	}
	return unscalePoint(maxP,eye);
}

#pragma mark Postprocessing

inline bool inMat(const cv::Point &np, const cv::Mat &mat) {
	return np.x >= 0 && np.x < mat.cols  &&  np.y >= 0 && np.y < mat.rows;
}

// returns a mask
cv::Mat floodKillEdges(cv::Mat &mat) {
	rectangle(mat,cv::Rect(0,0,mat.cols,mat.rows),255);
	
	cv::Mat mask(mat.rows, mat.cols, CV_8U, 255);
	std::queue<cv::Point> toDo;
	toDo.push(cv::Point(0,0));
	while (!toDo.empty()) {
		cv::Point p = toDo.front();
		toDo.pop();
		if (mat.at<float>(p) == 0.0f) {
			continue;
		}
		// add in every direction
		cv::Point np(p.x + 1, p.y); // right
		if (inMat(np, mat)) toDo.push(np);
		np.x = p.x - 1; np.y = p.y; // left
		if (inMat(np, mat)) toDo.push(np);
		np.x = p.x; np.y = p.y + 1; // down
		if (inMat(np, mat)) toDo.push(np);
		np.x = p.x; np.y = p.y - 1; // up
		if (inMat(np, mat)) toDo.push(np);
		// kill it
		mat.at<float>(p) = 0.0f;
		mask.at<uchar>(p) = 0;
	}
	return mask;
}
