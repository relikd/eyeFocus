//
//  FrameReader.h
//  eyeFocus
//
//  Created by Oleg Geier on 07/02/17.
//
//

#ifndef FrameReader_h
#define FrameReader_h

#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

class FrameReader {
	cv::VideoCapture capture;
	
public:
	cv::Mat frame;
	const char* filePath = "live stream";
	const bool isVideoFile;
	short downScaling = 1;
	
	FrameReader(const char* path) : capture(cv::VideoCapture( path )), filePath(path), isVideoFile(true) { init(); };
	FrameReader(int camIndex) : capture(cv::VideoCapture( camIndex )), isVideoFile(false) { init(); };
	
	/** Automatically determine if argument is index or path to file */
	static FrameReader initWithArgv(const char* argv) {
		if (strlen(argv) > 2) {
			return FrameReader( argv );
		} else {
			long index = 0;
			index = strtol(argv, NULL, 10);
			return FrameReader( (int)index );
		}
	}
	
	/**
	 * Get downscaled gray image from video source and save to ivar frame
	 * @return True if read was successful. False if frame temporarily not available.
	 */
	bool readNext() {
		cv::Mat img;
		capture.read(img);
		
		if( img.empty() ) {
			if (isVideoFile)
				exit(EXIT_SUCCESS); // exit on EOF
			fputs(" --(!) No captured frame -- Break!\n", stderr);
			frame = cv::Mat::zeros(640, 480, CV_8UC1);
			return false;
		}
		
		if (downScaling > 1)
			cv::resize( img, img, cv::Size(img.cols / downScaling, img.rows / downScaling) );
		
		cv::flip(img, img, 1); // mirror it
		
		// get gray image from blue channel
		std::vector<cv::Mat> rgbChannels(3);
		cv::split(img, rgbChannels);
		frame = rgbChannels[2];
		return true;
	}
	
private:
	void init() {
		if( !capture.isOpened() )
			exit(EXIT_FAILURE);
	}
};

#endif /* FrameReader_h */
