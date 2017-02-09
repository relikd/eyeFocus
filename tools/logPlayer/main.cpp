#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

bool grabDownscaledGreyFrame(cv::VideoCapture capture, cv::Mat *frame) {
	cv::Mat img;
	capture.read(img);
	
	if( img.empty() ) {
		return false;
	}
	
	cv::resize(img, img, cv::Size(img.cols/2, img.rows/2));
	cv::flip(img, img, 1); // mirror it
	
	// get gray image from blue channel
	std::vector<cv::Mat> rgbChannels(3);
	cv::split(img, rgbChannels);
	*frame = rgbChannels[2];
	return true;
}

int main( int argc, const char** argv )
{
	if (argc != 2) {
		fputs("Missing argument value. Pass either [path to video file] or [camera index].\n\n", stderr);
		return EXIT_SUCCESS;
	}
	char logFile[1024];
	strcpy(logFile, argv[1]);
	strcat(logFile, ".pupilpos.csv");
	FILE* file = fopen(logFile, "r");
	
	if (!file) {
		fputs("Log file not found '*.pupilpos.csv'.\n\n", stderr);
		return EXIT_FAILURE;
	}
	
	cv::namedWindow(argv[1], CV_WINDOW_NORMAL);
	cv::moveWindow(argv[1], 400, 100);
	
	// create Video Capture from calling argument
	cv::VideoCapture capture( argv[1] );
	if( !capture.isOpened() )
		return EXIT_FAILURE;
	
	fscanf(file, "%*[^\n]\n", NULL); // skip header row
	
	cv::Mat frame_gray;
	while ( grabDownscaledGreyFrame(capture, &frame_gray) )
	{
		float val[10];
		fscanf(file, "%f,%f,%f,%f,%f,%f,%f,%f,%f,%f\n", &val[0],&val[1],&val[2],&val[3],&val[4],&val[5],&val[6],&val[7],&val[8],&val[9]);
		
		circle(frame_gray, cv::Point2f(val[0],val[1]), 3, 1234);
		circle(frame_gray, cv::Point2f(val[2],val[3]), 3, 1234);
		drawMarker(frame_gray, cv::Point2f(val[5],val[6]), 200);
		drawMarker(frame_gray, cv::Point2f(val[7],val[8]), 200);
		imshow(argv[1], frame_gray);
		
		if (cv::waitKey(20) == 27) {
			break;
		}
	}
	
	fclose(file);
	
	return EXIT_SUCCESS;
}

