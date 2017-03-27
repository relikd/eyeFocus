//
//  setupDualCam.h
//  eyeFocus
//
//  Created by Oleg Geier on 12/02/17.
//
//

#ifndef setupDualCam_h
#define setupDualCam_h

#include <opencv2/imgproc/imgproc.hpp>

namespace Setup {
	class DualCam {
	public:
		DualCam(const char *path, const char* file);
	};
}

#endif /* setupDualCam_h */
