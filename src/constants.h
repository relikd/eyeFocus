#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <opencv2/imgproc/imgproc.hpp>

// Ask user to select eye reagion upfront. Otherwise find eyes with face detection
#define kCameraIsHeadmounted 1
#define kSetupEyeCoordinateSpace 0

typedef std::pair<cv::Point2f, cv::Point2f> PointPair;
typedef std::pair<cv::Rect2f, cv::Rect2f> RectPair;


const cv::String window_name_main = "Capture - Face detection";
const cv::String window_name_face = "Capture - Face";
const cv::String window_name_left_eye  = "Left Eye";
const cv::String window_name_right_eye = "Right Eye";


// Size constants
const int kEyePercentTop = 25;
const int kEyePercentSide = 13;
const int kEyePercentHeight = 30;
const int kEyePercentWidth = 35;

// Preprocessing
const bool kSmoothFaceImage = false;
const float kSmoothFaceFactor = 0.005;

// Algorithm Parameters
const bool kScaledownEyeImage = true;
const int kFastEyeWidth = 50;
const int kWeightBlurSize = 5;
const bool kEnableWeight = true;
const float kWeightDivisor = 1.0;
const double kGradientThreshold = 50.0;

// Postprocessing
const bool kEnablePostProcess = true;
const float kPostProcessThreshold = 0.97;

// Eye Corner
const bool kEnableEyeCorner = false;

// Smooth eye position over time
const bool kUseKalmanFilter = true;
const float kKalmanMeasureError = 150;
const float kKalmanProcessError = 1e-7;
const float kKalmanInitialError = 100000; // very large to jump to first position immediately

#endif
