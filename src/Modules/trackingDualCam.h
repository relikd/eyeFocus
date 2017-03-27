//
//  trackingDualCam.h
//  eyeFocus
//
//  Created by Oleg Geier on 12/02/17.
//
//

#ifndef trackingDualCam_h
#define trackingDualCam_h

#include <opencv2/imgproc/imgproc.hpp>

namespace Tracking {
	class DualCam {
	public:
		DualCam(const char *path, const char* file);
	};
}

#endif /* trackingDualCam_h */
