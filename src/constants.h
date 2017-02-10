#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <opencv2/imgproc/imgproc.hpp>

// Ask user to select eye reagion upfront. Otherwise find eyes with face detection
#define kCameraIsHeadmounted 1
#define kFullsizeSingleEyeMode 0
#define kEnableImageWindow 1 // disable all image output to improve processing performance

const cv::String window_setup_headmount = "Select eye area and corners";
const cv::String window_setup_single_eye = "Monokular Headmounted Setup";

// Size constants for face to eye aspect ratios
const int kEyePercentTop = 25;
const int kEyePercentSide = 13;
const int kEyePercentHeight = 30;
const int kEyePercentWidth = 35;

// Preprocessing, before pupil detection
const bool kSmoothFaceImage = false;
const float kSmoothFaceFactor = 0.005F;

// Timm Algorithm Parameters
const bool kScaledownEyeImage = true;
const int kFastEyeWidth = 50;
const int kWeightBlurSize = 5;
const bool kEnableWeight = true;
const float kWeightDivisor = 1.0;
const double kGradientThreshold = 50.0;

// Postprocessing, remove edges connected to border (like eye brows)
const bool kEnablePostProcess = true;
const float kPostProcessThreshold = 0.97F;

// Eye corner detection
const int kEyeCornerSearchArea = 30;
const int kEyeCornerDistanceInMM = 35; // 3.5cm

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
