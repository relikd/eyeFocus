#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <opencv2/imgproc/imgproc.hpp>

// Ask user to select eye reagion upfront. Otherwise find eyes with face detection
#define kCameraIsHeadmounted 1
#define kSetupEyeCoordinateSpace 0
#define kFullsizeSingleEyeMode 0 // kCameraIsHeadmounted must be 1

typedef std::pair<cv::Point2f, cv::Point2f> PointPair;
typedef std::pair<cv::Rect2i, cv::Rect2i> RectPair;
typedef std::pair<cv::RotatedRect, cv::RotatedRect> EllipsePair;


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
const float kSmoothFaceFactor = 0.005F;

// Algorithm Parameters
const bool kScaledownEyeImage = true;
const int kFastEyeWidth = 50;
const int kWeightBlurSize = 5;
const bool kEnableWeight = true;
const float kWeightDivisor = 1.0;
const double kGradientThreshold = 50.0;

// Postprocessing
const bool kEnablePostProcess = true;
const float kPostProcessThreshold = 0.97F;

// Eye Corner
const bool kEnableEyeCorner = false;
const int kEyeCornerSearchArea = 30;

// Smooth eye position over time
const bool kUseKalmanFilter = true;
const float kKalmanInitialError = 100000; // very large to jump to first found position immediately

#if kFullsizeSingleEyeMode
// lower kalman configuration to enable real time tracking
const float kKalmanMeasureError = 50;
const float kKalmanProcessError = 1e-3f;
#else
const float kKalmanMeasureError = 150;
const float kKalmanProcessError = 1e-7f;
#endif

#endif
