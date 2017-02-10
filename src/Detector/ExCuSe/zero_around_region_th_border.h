
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/core/core.hpp>
#include "../../constants.h"

/*
 Version 1.0, 08.06.2015, Copyright University of Tübingen.
 
 The Code is created based on the method from the paper:
 "ExCuSe: Robust Pupil Detection in Real-World Scenarios", W. Fuhl, T. C. Kübler, K. Sippel, W. Rosenstiel, E. Kasneci
 CAIP 2015 : Computer Analysis of Images and Patterns
 
 The code and the algorithm are for non-comercial use only.
 
 */


/*
 Sends out rays and collects lines for ellipse fit
 
 input:
 cv::Mat *th_edges ->
 int end_x -> ray length in x direction
 int end_y -> ray length in y direction
 cv::Point *pos -> starting position
 int *ret -> array of size 8 in which the found line index is stored
 
 */
static void rays(cv::Mat th_edges, int end_x, int end_y, cv::Point *pos, int *ret) {
	
	for (int i = 0; i<8; i++) ret[i] = -1;
	
	
	for (int i = 0; i<end_x; i++)
		for (int j = 0; j<end_y; j++) {
			
			if (pos->x-i>0 && pos->x+i<th_edges.cols && pos->y-j>0 && pos->y+j<th_edges.rows) {
				
				
				if ((int)th_edges.at<uchar>(pos->y, pos->x+i) != 0 && ret[0] == -1) {
					ret[0] = th_edges.at<uchar>(pos->y, pos->x+i)-1;
				}
				if ((int)th_edges.at<uchar>(pos->y, pos->x-i) != 0 && ret[1] == -1) {
					ret[1] = th_edges.at<uchar>(pos->y, pos->x-i)-1;
				}
				if ((int)th_edges.at<uchar>(pos->y+j, pos->x) != 0 && ret[2] == -1) {
					ret[2] = th_edges.at<uchar>(pos->y+j, pos->x)-1;
				}
				if ((int)th_edges.at<uchar>(pos->y-j, pos->x) != 0 && ret[3] == -1) {
					ret[3] = th_edges.at<uchar>(pos->y-j, pos->x)-1;
				}
				
				
				if ((int)th_edges.at<uchar>(pos->y+j, pos->x+i) != 0 && ret[4] == -1 && i == j) {
					ret[4] = th_edges.at<uchar>(pos->y+j, pos->x+i)-1;
				}
				if ((int)th_edges.at<uchar>(pos->y-j, pos->x-i) != 0 && ret[5] == -1 && i == j) {
					ret[5] = th_edges.at<uchar>(pos->y-j, pos->x-i)-1;
				}
				if ((int)th_edges.at<uchar>(pos->y-j, pos->x+i) != 0 && ret[6] == -1 && i == j) {
					ret[6] = th_edges.at<uchar>(pos->y-j, pos->x+i)-1;
				}
				if ((int)th_edges.at<uchar>(pos->y+j, pos->x-i) != 0 && ret[7] == -1 && i == j) {
					ret[7] = th_edges.at<uchar>(pos->y+j, pos->x-i)-1;
				}
				
				
			}
			
		}
	
	
}



/*
 Sends out rays and collects lines for ellipse fit
 
 input:
 cv::Mat *pic -> gray scale image
 cv::Mat *edges -> edge image
 cv::Mat *th_edges -> zero mat
 int th -> intesity threshold
 int edge_to_th -> max distance of edges to threasholded pixels
 double mean_dist -> line distance to mean for separation of curved and straight lines
 double area -> percentage value of window size
 cv::RotatedRect *pos -> the found ellipse
 
 */

static void zero_around_region_th_border(cv::Mat *pic, cv::Mat *edges, cv::Mat *th_edges, int th, int edge_to_th, double mean_dist, double area, cv::RotatedRect *pos) {
	
	int ret[8];
	std::vector<cv::Point> selected_points;
	cv::RotatedRect ellipse;
	
	int start_x = pos->center.x - (area*pic->cols);
	int end_x = pos->center.x + (area*pic->cols);
	int start_y = pos->center.y - (area*pic->rows);
	int end_y = pos->center.y + (area*pic->rows);
	
	
	if (start_x<0) start_x = edge_to_th;
	if (start_y<0) start_y = edge_to_th;
	if (end_x>pic->cols) end_x = pic->cols-(edge_to_th+1);
	if (end_y>pic->rows) end_y = pic->rows-(edge_to_th+1);
	
	th = th+th+1;
	
	
	for (int i = start_x; i<end_x; i++)
		for (int j = start_y; j<end_y; j++) {
			
			if (pic->at<uchar>(j,i) < th) {
				
				for (int k1 = -edge_to_th; k1 < edge_to_th; k1++)
					for (int k2 = -edge_to_th; k2 < edge_to_th; k2++) {
						
						if (i+k1 >= 0 && i+k1 < pic->cols && j+k2 > 0 && j+k2 < edges->rows)
							if (edges->at<uchar>(j+k2, i+k1))
								th_edges->at<uchar>(j+k2, i+k1) = 255;
						
					}
				
			}
			
		}
	
	std::vector<std::vector<cv::Point>> all_curves = get_curves(pic, th_edges, start_x, end_x, start_y, end_y, mean_dist, 0);
	
	for (std::vector<cv::Point> &vp : all_curves) {
		for (cv::Point &v : vp) {
			circle(*th_edges, v, 3, 1234);
		}
	}
//#if kEnableImageWindow
//	static bool tmpp;
//	if (tmpp)
//		imshow("test1", *th_edges);
//	else
//		imshow("test2", *th_edges);
//	tmpp = !tmpp;
//#endif
	
	if (all_curves.size()>0) {
		
		for (int i = 0; i<th_edges->cols; i++)
			for (int j = 0; j<th_edges->rows; j++) {
				th_edges->at<uchar>(j, i) = 0;
			}
		
		
		//draw remaining edges
		for (int i = 0; i<all_curves.size(); i++) {
			for (int j = 0; j<all_curves[i].size(); j++) {
				
				if (all_curves[i][j].x >= 0 && all_curves[i][j].x < th_edges->cols && all_curves[i][j].y >= 0 && all_curves[i][j].y < th_edges->rows)
					th_edges->at<uchar>(all_curves[i][j].y, all_curves[i][j].x) = i+1;//+1 becouse of first is 0
			}
		}
		
		
		cv::Point st_pos;
		st_pos.x = pos->center.x;
		st_pos.y = pos->center.y;
		//send rays add edges to vector
		rays(*th_edges, (end_x-start_x)/2, (end_y-start_y)/2, &st_pos, ret);
		
		
		//gather points
		for (int i = 0; i<8; i++)
			if (ret[i]>-1 && ret[i]<all_curves.size()) {
				for (int j = 0; j<all_curves[ret[i]].size(); j++) {
					selected_points.push_back(all_curves[ret[i]][j]);
				}
			}
		
		
		if (selected_points.size()>5) {
			
			*pos = cv::fitEllipse( cv::Mat(selected_points) );
			
		}
		
	}
	
//#if kEnableImageWindow
//	for (cv::Point &v : selected_points) {
//		circle(*th_edges, v, 3, 1234);
//	}
	
//	static bool tmpp;
//	if (tmpp)
//		imshow("test1", *th_edges);
//	else
//		imshow("test2", *th_edges);
//	tmpp = !tmpp;
//#endif
}
