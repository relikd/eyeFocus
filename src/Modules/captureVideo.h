//
//  captureVideo.h
//  eyeFocus
//
//  Created by Oleg Geier on 26/03/17.
//
//

#ifndef captureVideo_h
#define captureVideo_h

#include <opencv2/imgproc/imgproc.hpp>

class CaptureVideo {
public:
	static void dualCam(int startFocusCM = 80); // counting downwards in 10cm steps
	static void singleCam(int cam, int startFocusCM = 80);
	
private:
	static void waitForInput();
	static void writeCameraStreamToDisk(int cam, const char* subfolder = NULL);
	static std::string createDir(const char* basename, int focusLevel, const char* subfolder = NULL);
};

#endif /* captureVideo_h */
