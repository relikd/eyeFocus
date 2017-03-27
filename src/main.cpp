#include <opencv2/highgui/highgui.hpp>
#include "constants.h"
#include "Helper/FrameReader.h"
#include "Modules/captureVideo.h"
#include "Modules/trackingHeadmount.h"
#include "Modules/trackingSingleCam.h"
#include "Modules/trackingDualCam.h"

int main( int argc, const char** argv ) {
	
	CaptureVideo::singleCam(0, 80);
	return EXIT_SUCCESS;
	
#if 0
	Setup::DualCam::writeStreamToDisk(80);
	return EXIT_SUCCESS;
#endif
	
#if kFullsizeDualCamMode
	Tracking::DualCam(NULL, NULL);
//	for (int i=10; i<=80; i+=10) {
//		char file[20];
//		snprintf(file, 20*sizeof(char), "%dcm.MP4", i);
//		Tracking::DualCam("../../testVideos/series14/", file);
//	}
	return EXIT_SUCCESS;
#endif
	
	
	if (argc != 2) {
		fputs("Missing argument value. Pass either [path to video file] or [camera index].\n\n", stderr);
		return EXIT_SUCCESS;
	}
	
	// create Video Capture from calling argument
	FrameReader fr = FrameReader::initWithArgv(argv[1]);
	fr.downScaling = 2; // reduce size for GoPro
	
#if kFullsizeSingleEyeMode
	Tracking::SingleCam().start(fr); // will never return
	return EXIT_SUCCESS;
#endif
	
	Tracking::Headmount().start(fr);
	return EXIT_SUCCESS;
}
