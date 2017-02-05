
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/core/core.hpp>

#include "peek.h"
#include "find_best_edge.h"
#include "th_angular_histo.h"

#include "optimize_pos.h"
#include "zero_around_region_th_border.h"


#include "canny_ml.h"




/*
  Version 1.0, 08.06.2015, Copyright University of Tübingen.

  The Code is created based on the method from the paper:
  "ExCuSe: Robust Pupil Detection in Real-World Scenarios", W. Fuhl, T. C. Kübler, K. Sippel, W. Rosenstiel, E. Kasneci
  CAIP 2015 : Computer Analysis of Images and Patterns
 
  The code and the algorithm are for non-comercial use only.

*/


/*
Start function of the algorithm

input:
cv::Mat *pic -> gray scale image
cv::Mat *pic_th -> zero mat
cv::Mat *th_edges-> zero mat
bool matlab -> true: matlab canny		false:opencv canny


return:
cv::RotatedRect -> found ellipse. If only coarse positioning was succsessfull ellipse has no size. Center is zero if no pupil was found.

*/

static cv::RotatedRect run(cv::Mat pic, cv::Mat *pic_th, cv::Mat *th_edges, bool matlab){

	cv::Mat internalPic;
	cv::normalize(pic, internalPic, 0, 255, cv::NORM_MINMAX, CV_8U);



	double border=0.1;
	int peek_detector_factor=10;
	int bright_region_th=199;
	double mean_dist=3;
	int inner_color_range=5; 
	double th_histo=0.5;
	int max_region_hole=5;
	int min_region_size=7;
	double area_opt=0.1;
    double area_edges=0.2; 
	int edge_to_th=5;

	cv::RotatedRect ellipse;
	cv::Point pos(0,0);

	int start_x=floor(double(internalPic.cols)*border);
	int start_y=floor(double(internalPic.rows)*border);

	int end_x =internalPic.cols-start_x;
	int end_y =internalPic.rows-start_y;



	double stddev=0;
	bool edges_only_tried=false;
	bool peek_found=false;
	int threshold_up=0;


	peek_found=peek(&internalPic, &stddev, start_x, end_x, start_y, end_y, peek_detector_factor, bright_region_th);
	threshold_up=ceil(stddev/2);
	threshold_up--;

	


	cv::Mat picpic = cv::Mat::zeros(end_y-start_y, end_x-start_x, CV_8U);




	for(int i=0; i<picpic.cols; i++)
		for(int j=0; j<picpic.rows; j++){
			picpic.at<uchar>(j, i) = internalPic.at<uchar>(start_y+j, start_x+i);
		}
	
	
	cv::Mat detected_edges2;
	if(matlab){
		detected_edges2 = canny_impl(&picpic);
	}else{
		cv::GaussianBlur(picpic,detected_edges2, cv::Size(15,15),sqrt(2.0));
		Canny( detected_edges2, detected_edges2, stddev*0.4, stddev, 3 );
	}
	
	
	

	cv::Mat detected_edges = cv::Mat::zeros(internalPic.rows, internalPic.cols, CV_8U);
	for(int i=0; i<detected_edges2.cols; i++)
		for(int j=0; j<detected_edges2.rows; j++){
			detected_edges.at<uchar>(start_y+j, start_x+i) = detected_edges2.at<uchar>(j,i);
		}



	remove_points_with_low_angle(detected_edges, start_x, end_x, start_y, end_y);

	


	if(peek_found){
		edges_only_tried=true;
		ellipse=find_best_edge(&internalPic, &detected_edges, start_x, end_x, start_y, end_y,mean_dist, inner_color_range);

		if(ellipse.center.x<=0 || ellipse.center.x>=internalPic.cols || ellipse.center.y<=0 || ellipse.center.y>=internalPic.rows){
			ellipse.center.x=0;
			ellipse.center.y=0;
			ellipse.angle=0.0;
			ellipse.size.height=0.0;
			ellipse.size.width=0.0;
			peek_found=false;
		}
	}
	


	if(!peek_found){
		pos= th_angular_histo(&internalPic, pic_th, start_x, end_x, start_y, end_y, threshold_up, th_histo,max_region_hole, min_region_size);

		ellipse.center.x=pos.x;
		ellipse.center.y=pos.y;
		ellipse.angle=0.0;
		ellipse.size.height=0.0;
		ellipse.size.width=0.0;

	}
	

	if(pos.x==0 && pos.y==0 && !edges_only_tried){
		ellipse=find_best_edge(&internalPic, &detected_edges, start_x, end_x, start_y, end_y,mean_dist, inner_color_range);

		peek_found=true;
	}
	
	



	if(pos.x>0 && pos.y>0 && pos.x<internalPic.cols && pos.y<internalPic.rows && !peek_found){
		optimize_pos(&internalPic, area_opt, &pos);

		ellipse.center.x=pos.x;
		ellipse.center.y=pos.y;
		ellipse.angle=0.0;
		ellipse.size.height=0.0;
		ellipse.size.width=0.0;
		zero_around_region_th_border(&internalPic, &detected_edges, th_edges, threshold_up, edge_to_th, mean_dist, area_edges, &ellipse);
	}
	
	//	cv::ellipse(internalPic, ellipse, 1243);
//	cv::circle(internalPic, ellipse.center, 3, 1242);
//	imshow("one", detected_edges);
//	imshow("two", detected_edges2);
//	imshow("threes", internalPic);
//	cv::waitKey(10);

	return ellipse;
	
}








