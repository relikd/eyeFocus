//
//  captureVideo.cpp
//  eyeFocus
//
//  Created by Oleg Geier on 26/03/17.
//
//

#include "captureVideo.h"
#include <opencv2/highgui/highgui.hpp>
#include <thread>
#include "../Helper/FileIO.h"

static int currentFocusLevel = 5;

void CaptureVideo::dualCam(int startFocusCM) {
	currentFocusLevel = startFocusCM + 5; // start with inactive phase
	
	bool bothInSync = false;
	std::thread a([&bothInSync]{
		while (!bothInSync) { /* nothing */ }
		writeCameraStreamToDisk(0, "0");
	});
	std::thread b([&bothInSync]{
		while (!bothInSync) { /* nothing */ }
		writeCameraStreamToDisk(1, "1");
	});
	bothInSync = true;
	waitForInput();
	a.join();
	b.join();
}

void CaptureVideo::singleCam(int cam, int startFocusCM) {
	currentFocusLevel = startFocusCM + 5; // start with inactive phase
	std::thread a([cam]{
		writeCameraStreamToDisk(cam);
	});
	waitForInput();
	a.join();
}

void CaptureVideo::waitForInput() {
	FileIO::createDirectory("Recording");
	
	cv::String window = "Press space to move 5cm closer (ESC abort)";
	cv::namedWindow(window, CV_WINDOW_NORMAL);
	cv::moveWindow(window, 150, 100);
	imshow(window, cv::Mat::zeros(480, 640, CV_8UC1));
	
	printf("Status: ready. Press spacebar to move 5 cm closer.\n");
	while (true) {
		switch (cv::waitKey(10)) {
			case 27: // ESC
				currentFocusLevel = -1;
				return;
			case ' ':
				currentFocusLevel -= 5; // each 5cm step is inactive phase
				printf("Level: %d\n", currentFocusLevel);
		}
	}
}

void CaptureVideo::writeCameraStreamToDisk(int cam, const char* subfolder) {
	cv::VideoCapture vc(cam);
//	double width = vc.get(CV_CAP_PROP_FRAME_WIDTH);
//	double height = vc.get(CV_CAP_PROP_FRAME_HEIGHT);
//	double fps = vc.get(CV_CAP_PROP_FPS);
	bool directoryCreated = false;
	const char* basePath;
	
	cv::Mat img;
	size_t f = 0;
	while (++f && currentFocusLevel != -1)
	{
		if (currentFocusLevel % 10 == 5) { // skip recording and reset counter
			f = 0;
			directoryCreated = false;
			continue;
		}
		
		if (!directoryCreated) {
			basePath = createDir("Recording", currentFocusLevel, subfolder).c_str();
			directoryCreated = true;
		}
		
		vc.read(img);
		imwrite(FileIO::str("%s/frame_%lu.jpg", basePath, f), img);
	}
}

std::string CaptureVideo::createDir(const char* basename, int focusLevel, const char* subfolder) {
	const char* basepath;
	if (subfolder && strlen(subfolder) > 0) {
		basepath = FileIO::str("%s/%s", basename, subfolder).c_str();
		FileIO::createDirectory(basepath);
	} else {
		basepath = FileIO::str("%s", basename).c_str();
	}
	std::string fullpath = FileIO::str("%s/%d", basepath, currentFocusLevel);
	FileIO::createDirectory(fullpath.c_str());
	return fullpath;
}
