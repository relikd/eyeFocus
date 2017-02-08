//
//  setupHeadmount.h
//  eyeFocus
//
//  Created by Oleg Geier on 26/12/16.
//
//

#ifndef SETUP_HEADMOUNT_H
#define SETUP_HEADMOUNT_H

#include "../Helper/FrameReader.h"

namespace Setup {
	class Headmount {
	public:
		cv::Rect2i leftEyeBox, rightEyeBox;
		cv::Rect2i leftEyeCorner, rightEyeCorner;
		
		Headmount(FrameReader fr, const char* savePath = NULL);
	};
}

#endif /* SETUP_HEADMOUNT_H */
