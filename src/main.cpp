#include <opencv2/highgui/highgui.hpp>
#include "constants.h"
#include "Helper/FrameReader.h"
#include "Modules/captureVideo.h"
#include "Modules/trackingHeadmount.h"
#include "Modules/trackingSingleCam.h"
#include "Modules/trackingDualCam.h"

int menuSelector(std::vector<std::string> strings);

int main( int argc, const char** argv )
{
	bool downscale = false;
	short index = -1;
	short argindex = 0;
	const char* argfile[2] = { NULL, NULL };
	
	for (int i = 1; i < argc; i++) {
		const char* param = argv[i];
		if (strcmp(param, "-h") == 0 || strcmp(param, "-half") == 0)
			downscale = true;
		else if (param[0] == '-')
			index = param[1] - 48;
		else if (argindex < 2)
			argfile[argindex++] = param;
	}
	
//	for (int i=10; i<=80; i+=10) {
//		char file[20];
//		snprintf(file, 20*sizeof(char), "%dcm.MP4", i);
//		Tracking::DualCam("../../testVideos/series14/", file);
//	}
	
	while (true) {
		if (index == -1) {
			std::vector<std::string> menuStrings;
			menuStrings.push_back("Capture Video 1 Cam");
			menuStrings.push_back("Capture Video 2 Cam");
			menuStrings.push_back("Face Detection");
			menuStrings.push_back("Headmounted Tracking");
			menuStrings.push_back("Single Cam Tracking");
			menuStrings.push_back("Dual Cam Tracking");
			index = menuSelector(menuStrings);
		}
		
		
		if (index >= 2 && index <= 4) {
			// create Video Capture from calling argument
			FrameReader fr = FrameReader::initWithArgv( argfile[0] ? argfile[0] : "0" );
			if (downscale)
				fr.downScaling = 2; // reduce size for GoPro
			
			switch (index) {
				case 2: Tracking::Headmount().start(fr, true); break;
				case 3: Tracking::Headmount().start(fr, false); break;
				case 4: Tracking::SingleCam().start(fr); break;
			}
		} else {
			switch (index) {
				case 0: CaptureVideo::singleCam(0, 80); break;
				case 1: CaptureVideo::dualCam(80); break;
				case 5: Tracking::DualCam(argfile[0], argfile[1]); break;
			}
		}
		
		index = -1;
	}
	
	return EXIT_SUCCESS;
}

int menuSelector(std::vector<std::string> strings) {
	cv::destroyAllWindows();
	cv::waitKey(1);
	cv::namedWindow("Menu", CV_WINDOW_NORMAL);
	cv::moveWindow("Menu", 370, 200);
	
	int index = 0;
	while (true) {
		cv::Mat menu = cv::Mat::zeros(240, 480, CV_8UC1);
		
		float yoffset = 10;
		for (int i = 0; i < strings.size(); i++) {
			std::string str = (i==index ? ">" : " ") + strings[i];
			cv::Size s = cv::getTextSize(str, cv::FONT_HERSHEY_PLAIN, 2.0f, 1, NULL);
			yoffset += s.height + 7;
			cv::putText(menu, str, cv::Point(10, yoffset), cv::FONT_HERSHEY_PLAIN, 2.0f, 255);
		}
		
		imshow("Menu", menu);
		switch (cv::waitKey(0)) {
			case 27: // ESC
				exit(EXIT_SUCCESS);
			case 1: // arrow down
			case 3: // arrow right
				index++; break;
			case 0: // arrow up
			case 2: // arrow left
				index--; break;
			case 13: // Return
				cv::destroyAllWindows();
				cv::waitKey(1);
				return index;
		}
		
		if (index < 0)
			index += strings.size(); // why is   (-1) % 6 == 3   and not 5
		index %= strings.size();
	}
	return -1;
}
