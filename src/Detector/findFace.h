//
//  findFace.h
//  eyeFocus
//
//  Created by Oleg Geier on 26/12/16.
//
//

#ifndef findFace_h
#define findFace_h

#include "../constants.h"
#include <opencv2/objdetect/objdetect.hpp>

namespace Detector {
	class Face {
		cv::CascadeClassifier face_cascade;
		
	public:
		/** @param name Path to cascade file */
		Face(const cv::String name) {
			// Load the cascades
			if ( !face_cascade.load( name ) ) {
				printf("--(!)Error loading face cascade, please change face_cascade_name in source code.\n");
				exit(EXIT_FAILURE);
			}
		};
		
		/** Get both eye regions. @return face rect */
		cv::Rect2i find(const cv::Mat &frame, cv::Rect2i* leftEye, cv::Rect2i* rightEye) {
			//cvtColor( frame, frame_gray, CV_BGR2GRAY );
			//equalizeHist( frame_gray, frame_gray );
			//cv::pow(frame_gray, CV_64F, frame_gray);
			
			cv::Rect face_r = cv::Rect();
			
			//-- Detect faces
			std::vector<cv::Rect> faces;
			// Apply the classifier to the frame
			face_cascade.detectMultiScale( frame, faces, 1.1, 2, 0|CV_HAAR_SCALE_IMAGE|CV_HAAR_FIND_BIGGEST_OBJECT, cv::Size(150, 150) );
			if (faces.size() > 0)
				face_r = faces[0];
			
			//-- Detect eye box
			*leftEye  = eyeRegionForFace(face_r, true) + face_r.tl();
			*rightEye = eyeRegionForFace(face_r, false) + face_r.tl();
			return face_r;
		};
		
	private:
		cv::Rect2i eyeRegionForFace(cv::Rect2i face, bool isLeftEye) {
			int eye_region_width = (face.width * kEyePercentWidth)/100;
			int eye_region_height = (face.width * kEyePercentHeight) / 100;
			int eye_region_top = (face.height * kEyePercentTop) / 100;
			cv::Rect2i region(0, eye_region_top, eye_region_width, eye_region_height);
			
			if (isLeftEye) region.x = (face.width * kEyePercentSide) / 100;
			else           region.x = face.width - eye_region_width - (face.width * kEyePercentSide) / 100;
			
			return region;
		};
	};
}

#endif /* findFace_h */
