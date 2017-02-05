#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/core/core.hpp>


#include "remove_points_with_low_angle.h"
#include "get_curves.h"


static cv::RotatedRect find_best_edge(cv::Mat *pic,cv::Mat *edge, int start_x, int end_x, int start_y, int end_y, double mean_dist, int inner_color_range){

	cv::RotatedRect ellipse;
	ellipse.center.x=0;
	ellipse.center.y=0;
	ellipse.angle=0.0;
	ellipse.size.height=0.0;
	ellipse.size.width=0.0;

	std::vector<std::vector<cv::Point>> all_curves=get_curves(pic, edge, start_x, end_x, start_y, end_y, mean_dist, inner_color_range);





	if(all_curves.size()==1){
		ellipse=cv::fitEllipse( cv::Mat(all_curves[0]) );

		if(ellipse.center.x<0 || ellipse.center.y<0 || ellipse.center.x>pic->cols || ellipse.center.y>pic->rows){
			ellipse.center.x=0;
			ellipse.center.y=0;
			ellipse.angle=0.0;
			ellipse.size.height=0.0;
			ellipse.size.width=0.0;
		}

	}else{
		ellipse.center.x=0;
		ellipse.center.y=0;
		ellipse.angle=0.0;
		ellipse.size.height=0.0;
		ellipse.size.width=0.0;
	}

	return ellipse;

}