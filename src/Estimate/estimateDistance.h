//
//  estimateDistance.h
//  eyeFocus
//
//  Created by Oleg Geier on 03/02/17.
//
//

#ifndef estimateDistance_hpp
#define estimateDistance_hpp

#include <opencv2/imgproc/imgproc.hpp>
#include <vector>

namespace Estimate {
	
	class Distance {
		int unknowns = 0;
		double* _x = new double[1];
		
	public:
		/** @param path Path to distance level configuration file */
		Distance() {};
		~Distance() {
			delete [] _x;
		}
		
		void initialize(std::vector<float> pupilDistance, std::vector<int> focusDistance);
		void initialize(int count, float* pupilDistance, int* focusDistance);
		
		double estimate(float pplDist);
		void printEquation(bool newline = true);
		
		static void drawOnFrame(cv::Mat &frame, double distance);
		
	private:
		inline int e(int index); // exponent calculation
	};
}

#endif /* estimateDistance_hpp */
