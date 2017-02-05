#ifndef estimateDistance_hpp
#define estimateDistance_hpp

#include <stdio.h>
#include <vector>
#include "../constants.h"

namespace Estimate {
	struct FocalLevel {
		int distance = 0;
		float min, avg, max;
	};
	
	
	class Distance {
		FILE* file;
		std::vector<FocalLevel> listDegrees;
		std::vector<FocalLevel> listRatios;
		
	public:
		Distance(bool initFromConfigFile = true) {
			if (initFromConfigFile) {
				readConfigFile("estimate.cfg");
			}
		};
		
		static int singlePupilHorizontal(float x, float cm20, float cm50, float cm80);
		int estimate(EllipsePair pupil, PointPair corner, bool byDegrees = true); // otherwise by ratio
		
	private:
		void readConfigFile(const char* path);
	};
}

#endif /* estimateDistance_hpp */
