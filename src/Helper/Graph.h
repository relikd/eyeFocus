//
//  Graph.h
//  eyeFocus
//
//  Created by Oleg Geier on 10/03/17.
//
//

#ifndef Graph_h
#define Graph_h

#include <opencv2/imgproc/imgproc.hpp>
#include <functional>
#include <vector>

class Graph {
	const int margin = 20;
	const cv::Point2i min, max;
	const cv::Point2f origin;
	const cv::Point2f pxPerUnit;
	
public:
	cv::Mat img;
	
	/**
	 * Generate new drawing space
	 * @param min Coordinate axis origin (function values)
	 * @param max Coordinate axis upper-right most corner (function values)
	 * @param size Resulting image size. Default: 640x480
	 */
	Graph(cv::Point2i min, cv::Point2i max, cv::Size2i size = cv::Size2i(640,480))
	:
	min(min), max(max),
	origin(cv::Point2f(margin, size.height - margin)),
	pxPerUnit(cv::Point2f((size.width - margin) / (float)(max.x - min.x),
						  (size.height - margin) / (float)(max.y - min.y))),
	img(cv::Mat::ones(size, CV_8SC1) * 255)
	{};
	~Graph() { img.release(); };
	
	/**
	 * Draw function y = f(x)
	 * @param step    Increment x by step size
	 * @param f       Function for plotting f(x)
	 * @param start   Used for clipping
	 * @param end     Used for clipping
	 * @param connect Draw a continuous line. Default: true
	 */
	Graph& addFunction(float step, std::function<double(float &x)> f, float start = 0, float end = 0, bool connect = true) {
		if (start == 0 && end == 0) {
			start = min.x;
			end = max.x;
		}
		bool first = true;
		cv::Point2f prevPoint;// = origin + cv::Point2f(0, -(f(tmp) - min.y) * pxPerUnit.y);
		for (float pos = start; pos < end; pos += step) {
			float x = pos;
			float off_y = -(f(x) - min.y) * pxPerUnit.y;
			float off_x = (x - min.x) * pxPerUnit.x;
			cv::Point2f currentPoint = origin + cv::Point2f(off_x, off_y);
			
			if (connect) {
				if (first) first = false; // only to save previous point
				else cv::line(img, prevPoint, currentPoint, 0);
				prevPoint = currentPoint;
			} else {
				cv::circle(img, currentPoint, 1, 0);
			}
		}
		return *this;
	}
	
	/** 
	 * Draw marker for measurement points
	 * @param points Vector of Point(x,y)
	 */
	Graph& addMarkers(std::vector<cv::Point2f> points) {
		for (cv::Point2f &pt : points) {
			float x = origin.x + (pt.x - min.x) * pxPerUnit.x;
			float y = origin.y - (pt.y - min.y) * pxPerUnit.y;
			cv::drawMarker(img, cv::Point(x,y), 0);
		}
		return *this;
	}
	
	/**
	 * Draw coordinate axis
	 * @param major     Major marks are larger and will be labeled. Must be a multiple of minor
	 * @param minor     Minor marks interval. To disable use same size as major.
	 * @param axisScale Multiply label values. Default: (1,1)
	 */
	Graph& addAxis(cv::Point2i major, cv::Point2i minor, cv::Point2f axisScale = cv::Point2f(1,1))
	{
		const float major_stroke = 10.0f;
		const float minor_stroke = 5.0f;
		// x-axis
		cv::arrowedLine(img, cv::Point(0, origin.y), cv::Point(img.cols, origin.y), 0, 1, 8, 0, 10.0/img.cols);
		for (int x = min.x; x < max.x; x++) {
			if (x % minor.x == 0 && x != min.x) {
				float offset = (x - min.x) * pxPerUnit.x;
				bool isMajor = (x % major.x == 0);
				if (isMajor)
					addAxisMarkerLabel(origin + cv::Point2f(offset, 15), x * axisScale.x, true);
				
				float mm = (isMajor ? major_stroke : minor_stroke) / 2;
				cv::line(img, origin + cv::Point2f(offset,-mm), origin + cv::Point2f(offset,mm), 0);
			}
		}
		// y-axis
		cv::arrowedLine(img, cv::Point(origin.x, img.rows), cv::Point(origin.x, 0), 0, 1, 8, 0, 10.0/img.rows);
		for (int y = min.y; y < max.y; y++) {
			if (y % minor.y == 0 && y != min.y) {
				float offset = -(y - min.y) * pxPerUnit.y;
				bool isMajor = (y % major.y == 0);
				if (isMajor)
					addAxisMarkerLabel(cv::Point2f(0, origin.y + offset), y * axisScale.y, false, true);
				
				float mm = (isMajor ? major_stroke : minor_stroke) / 2;
				cv::line(img, origin + cv::Point2f(-mm,offset), origin + cv::Point2f(mm,offset), 0);
			}
		}
		return *this;
	}
	
	/** Axis description near axis arrow */
	Graph& addAxisLabels(cv::String xAxis, cv::String yAxis) {
		addAxisLabel(xAxis, true);
		addAxisLabel(yAxis, false);
		return *this;
	}
	
private:
	void addAxisMarkerLabel(cv::Point pt, int number, bool centeringX = false, bool centeringY = false) {
		char label[10];
		snprintf(label, 10*sizeof(char), "%d", number);
		cv::Size s = cv::getTextSize(label, cv::FONT_HERSHEY_SIMPLEX, 0.3, 1, NULL);
		if (centeringX) pt.x -= s.width/2.0f;
		if (centeringY) pt.y += s.height/2.0f;
		cv::putText(img, label, pt, cv::FONT_HERSHEY_SIMPLEX, 0.3, 0);
	}
	void addAxisLabel(cv::String text, bool xAxis) {
		cv::Size s = cv::getTextSize(text, cv::FONT_HERSHEY_SIMPLEX, 0.3, 1, NULL);
		cv::Point p = cv::Point(img.cols - s.width - 5, s.height + 5);
		if (xAxis) p.y = origin.y - 10;
		else       p.x = origin.x + 10;
		cv::putText(img, text, p, cv::FONT_HERSHEY_SIMPLEX, 0.3, 0);
	}
};

#endif /* Graph_h */
