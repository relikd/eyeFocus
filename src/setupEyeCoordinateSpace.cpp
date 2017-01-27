#include "setupEyeCoordinateSpace.h"

#define PREV_WIDTH 100
#define PREV_HEIGHT 100

using namespace Setup;

std::vector<std::pair<cv::Mat, cv::Mat>> images;

const cv::Scalar SCALAR_WHITE = cv::Scalar(255.0, 255.0, 255.0);
const cv::String eye_coordinate_window = "Eye Coordinate System";

//  ---------------------------------------------------------------
// |
// |  Helper functions
// |
//  ---------------------------------------------------------------

cv::Mat smallerGrayscale(cv::Mat full, cv::Rect2f region, cv::Point2f pupil) {
	cv::Mat retImg;
	full(region).copyTo(retImg);
	// use this if image is rgb color
//	cv::cvtColor(retImg, retImg, CV_RGB2GRAY);
//	cv::cvtColor(retImg, retImg, CV_GRAY2RGB);
	cv::drawMarker(retImg, pupil - region.tl(), SCALAR_WHITE);
	cv::resize(retImg, retImg, cv::Size(PREV_WIDTH, PREV_HEIGHT));
	return retImg;
}

cv::Point2d relativePoint(cv::Point2f orig, cv::Rect2f rect) {
	cv::Point2d relative = orig - rect.tl();
	relative.x /= rect.width;
	relative.y /= rect.height;
	return relative;
}

void printPointPair(PointPair pp) {
	printf("<[%1.3f,%1.3f], [%1.3f,%1.3f]>\n", pp.first.x, pp.first.y, pp.second.x, pp.second.y);
}

double angleOfVector(cv::Point2d reference, cv::Point2d vec) {
	double dx = vec.x - reference.x;
	double dy = reference.y - vec.y;
	if (dx == 0) {
		if (dy > 0) return M_PI_2;
		else        return M_PI + M_PI_2;
	}
	double angle = atan( dy / dx );
	if (dx < 0) return angle + M_PI;
	if (dy < 0) return angle + M_PI * 2;
	return angle;
}


//  ---------------------------------------------------------------
// |
// |  Pair Sorting
// |
//  ---------------------------------------------------------------

struct {
	cv::Point2f center;
	bool operator() (PointPair a, PointPair b) {
		double angleA = angleOfVector(center, a.first);
		double angleB = angleOfVector(center, b.first);
		
		// rightmost should be first with an angle of 0 +/- 15° ( 0.2617 / 6.0214 )
		if (angleA > 6.0214) return true;
		if (angleB > 6.0214) return false;
		
		return (angleA < angleB);
	}
} SortByAngle ;

struct {
	cv::Point2f center;
	bool operator() (PointPair a, PointPair b) {
		return (cv::norm(a.first - center) < cv::norm(b.first - center) );
	}
} SortByEuclideanDistance ;


//  ---------------------------------------------------------------
// |
// |  All points collected
// |
//  ---------------------------------------------------------------

void EyeCoordinateSpace::sortPositions() {
	// Calculate average over all 9 points
	cv::Point2f avg;
	for (PointPair ppl : positions) {
		avg += ppl.first;
	}
	avg /= 9;
	
	// Find actual tracked center point
	SortByEuclideanDistance.center = avg;
	std::sort(positions.begin(), positions.end(), SortByEuclideanDistance);
	
	// Sort positions with the first item being the center, then rightmost, then all other counter clockwise
	SortByAngle.center = positions.front().first;
	std::sort(positions.begin()+1, positions.end(), SortByAngle);
}

bool EyeCoordinateSpace::waitForInput(cv::Mat image, RectPair region, PointPair pupil, cv::Point2f faceOffset)
{
	bool shouldRedraw = false;
	
	int key = cv::waitKey(2);
	if (key == ' ') // spacebar
	{
		// Test points
//		positions.push_back(std::make_pair( cv::Point2f(0.378, 0.235), cv::Point2f(0.422, 0.235) ));
//		positions.push_back(std::make_pair( cv::Point2f(0.519, 0.281), cv::Point2f(0.579, 0.281) ));
//		positions.push_back(std::make_pair( cv::Point2f(0.541, 0.351), cv::Point2f(0.602, 0.325) ));
//		positions.push_back(std::make_pair( cv::Point2f(0.343, 0.404), cv::Point2f(0.358, 0.377) ));
//		positions.push_back(std::make_pair( cv::Point2f(0.439, 0.398), cv::Point2f(0.538, 0.398) ));
//		positions.push_back(std::make_pair( cv::Point2f(0.659, 0.426), cv::Point2f(0.659, 0.400) ));
//		positions.push_back(std::make_pair( cv::Point2f(0.421, 0.518), cv::Point2f(0.383, 0.465) ));
//		positions.push_back(std::make_pair( cv::Point2f(0.520, 0.532), cv::Point2f(0.504, 0.514) ));
//		positions.push_back(std::make_pair( cv::Point2f(0.519, 0.559), cv::Point2f(0.581, 0.559) ));
		
		if (positions.size() == 9) {
			sortPositions();
			//for (PointPair p : positions) { printPointPair(p); }
			cv::destroyWindow(eye_coordinate_window);
			return true; // user setup complete
		}
		
		
		cv::Rect2f lr = region.first + faceOffset;
		cv::Rect2f rr = region.second + faceOffset;
		// if pupil found
		if (lr.tl() != pupil.first && rr.tl() != pupil.second) {
			images.push_back(std::make_pair( smallerGrayscale(image, lr, pupil.first), smallerGrayscale(image, rr, pupil.second) ));
			positions.push_back(std::make_pair( relativePoint(pupil.first, lr), relativePoint(pupil.second, rr) ));
			shouldRedraw = true;
		}
	}
	else if (key == 27) // esc key
	{
		if (positions.empty()) {
			cv::destroyWindow(eye_coordinate_window);
			//exit(EXIT_FAILURE);
			return true; // cancel setup
		}
		images.pop_back();
		positions.pop_back();
		shouldRedraw = true;
	}
	
	
	// User instructions
	cv::String infoText = "Press space to capture position (ESC undo)";
	if (positions.size() == 9) {
		infoText = "Press space to confirm selection";
	}
	cv::putText(image, infoText, cv::Point(10, image.rows - 10), cv::FONT_HERSHEY_PLAIN, 2.0f, SCALAR_WHITE);
	
	// draw debug points
//	for (PointPair p : positions) {
//		cv::Point2f pf = p.first;
//		pf.x *= region.first.size().width;
//		pf.y *= region.first.size().height;
//		cv::Point2f ps = p.second;
//		ps.x *= region.second.size().width;
//		ps.y *= region.second.size().height;
//		cv::circle(image, pf + region.first.tl() + faceOffset, 3, SCALAR_WHITE);
//		cv::circle(image, ps + region.second .tl() + faceOffset , 3, SCALAR_WHITE);
//	}
	
	// Horizontal eye line
#if !kCameraIsHeadmounted
	if (positions.size() < 9) {
		// extend horizontal line from one pupil to the other
		float ratio = (pupil.second.y - pupil.first.y) / (pupil.second.x - pupil.first.x);
		cv::Point2f a = cv::Point2f(0, pupil.first.y - pupil.first.x * ratio );
		cv::Point2f b = cv::Point2f(image.cols, pupil.second.y + (image.cols - pupil.second.x) * ratio );
		cv::line(image, a, b, SCALAR_WHITE);
		cv::putText(image, "Try to hold the line as horizontal as possible", cv::Point(10, a.y), cv::FONT_HERSHEY_PLAIN, 1.0f, SCALAR_WHITE);
	}
#endif
	
	// Display current selection in a separate window
	if (shouldRedraw) {
		cv::Mat tmp = cv::Mat::zeros(PREV_HEIGHT * 3, PREV_WIDTH * 6, CV_8UC1);
		for (int i = 0; i < images.size(); i++) {
			int offsetX = PREV_WIDTH * (i % 3);
			int offsetY = PREV_HEIGHT * (i / 3);
			cv::Rect clipping = cv::Rect(offsetX, offsetY, PREV_WIDTH, PREV_HEIGHT);
			
			images[i].first.copyTo(tmp(clipping));
//			clipping.x = (5 * PREV_WIDTH) - clipping.x; // display both images symmetrically flipped
			clipping.x += (3 * PREV_WIDTH); // display both images in same order
			images[i].second.copyTo(tmp(clipping));
		}
		cv::imshow(eye_coordinate_window, tmp);
	}
	
	return false;
}
