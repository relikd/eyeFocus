#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/core/core.hpp>

#define IMG_SIZE 1000 //maximal image size


/*
 Version 1.0, 08.06.2015, Copyright University of Tübingen.
 
 The Code is created based on the method from the paper:
 "ExCuSe: Robust Pupil Detection in Real-World Scenarios", W. Fuhl, T. C. Kübler, K. Sippel, W. Rosenstiel, E. Kasneci
 CAIP 2015 : Computer Analysis of Images and Patterns
 
 The code and the algorithm are for non-comercial use only.
 
 */

static uchar mean_inner_grey_from_point(cv::Point curve_pt, cv::Point mean_p, cv::Mat *pic) {
	int x = curve_pt.x;
	int y = curve_pt.y;
	double x_mean_p = x - (double)mean_p.x;
	double y_mean_p = y - (double)mean_p.y;
	if(pic->at<uchar>(y+1, x)!=0 || pic->at<uchar>(y-1, x)!=0 ) {
		if( sqrt( pow(y_mean_p, 2) + pow(x_mean_p+2, 2)) <
		   sqrt( pow(y_mean_p, 2) + pow(x_mean_p-2, 2)) ) {
			
			return pic->at<uchar>(y, x+2);
		} else {
			return pic->at<uchar>(y, x-2);
		}
		
	} else if(pic->at<uchar>(y, x+1)!=0 || pic->at<uchar>(y, x-1)!=0 ) {
		if( sqrt( pow(y_mean_p+2, 2) + pow(x_mean_p, 2)) <
		   sqrt( pow(y_mean_p-2, 2) + pow(x_mean_p, 2)) ) {
			
			return pic->at<uchar>(y+2, x);
		} else {
			return pic->at<uchar>(y-2, x);
		}
		
	} else if(pic->at<uchar>(y+1, x+1)!=0 || pic->at<uchar>(y-1, x-1)!=0 ) {
		if( sqrt( pow(y_mean_p-2, 2) + pow(x_mean_p+2, 2)) <
		   sqrt( pow(y_mean_p+2, 2) + pow(x_mean_p-2, 2)) ) {
			
			return pic->at<uchar>(y-2, x+2);
		} else {
			return pic->at<uchar>(y+2, x-2);
		}
		
	} else if(pic->at<uchar>(y-1, x+1)!=0 || pic->at<uchar>(y+1, x-1)!=0 ) {
		if( sqrt( pow(y_mean_p+2, 2) + pow(x_mean_p+2, 2)) <
		   sqrt( pow(y_mean_p-2, 2) + pow(x_mean_p-2, 2)) ) {
			
			return pic->at<uchar>(y+2, x+2);
		} else {
			return pic->at<uchar>(y-2, x-2);
		}
	}
	return 0;
}

/*
 Function which collects edges from the edge image
 
 input:
 cv::Mat *pic -> gray scale image
 cv::Mat *edge -> edge image
 int start_x -> start position in image x
 int end_x -> start position in image y
 int start_y -> end position in image x
 int end_y -> end position in image y
 double mean_dist -> line distance to mean for separation of curved and straight lines
 int inner_color_range -> range in which gray values are considered equal
 
 
 return:
 std::vector<std::vector<cv::Point>> -> If inner_color_range==0 all found edges are returnd else the best evaluated edge is returned
 
 */
static std::vector<std::vector<cv::Point>> get_curves(cv::Mat *pic, cv::Mat *edge, int start_x, int end_x, int start_y, int end_y, double mean_dist, int inner_color_range){
	
	std::vector<std::vector<cv::Point>> all_curves;
	std::vector<cv::Point> curve;
	
	if(start_x<2) start_x=2;
	if(start_y<2) start_y=2;
	if(end_x>pic->cols-2) end_x=pic->cols-2;
	if(end_y>pic->rows-2) end_y=pic->rows-2;
	
	
	int curve_idx=0;
	cv::Point mean_p;
	bool add_curve;
	int mean_inner_gray;
	int mean_inner_gray_last=1000000;
	
	
	all_curves.clear();
	
	bool check[IMG_SIZE][IMG_SIZE];
	
	for (int i=0; i<IMG_SIZE; i++)
		for (int j=0; j<IMG_SIZE; j++)
			check[i][j] = 0;
	
	
	for (int i=start_x; i<end_x; i++) {
		for (int j=start_y; j<end_y; j++) {
			
			if (edge->at<uchar>(j, i) == 255 && !check[i][j]) {
				check[i][j]=1;
				
				curve.clear();
				curve_idx=0;
				
				curve.push_back(cv::Point(i,j));
				mean_p.x=i;
				mean_p.y=j;
				curve_idx++;
				
				
				int akt_idx=0;
				
				while(akt_idx<curve_idx){
					
					cv::Point akt_pos=curve[akt_idx];
					for(int k1=-1;k1<2;k1++)
						for(int k2=-1;k2<2;k2++){
							
							if(akt_pos.x+k1>=start_x && akt_pos.x+k1<end_x && akt_pos.y+k2>=start_y && akt_pos.y+k2<end_y)
								if(!check[akt_pos.x+k1][akt_pos.y+k2] )
									if( edge->at<uchar>(akt_pos.y+k2, akt_pos.x+k1) == 255){
										check[akt_pos.x+k1][akt_pos.y+k2]=1;
										
										mean_p.x+=akt_pos.x+k1;
										mean_p.y+=akt_pos.y+k2;
										curve.push_back(cv::Point(akt_pos.x+k1, akt_pos.y+k2));
										curve_idx++;
									}
						}
					akt_idx++;
					
				}
				
				if (curve_idx>0 && curve.size()>0) {
					
					add_curve=true;
					mean_p.x=floor(( double(mean_p.x)/double(curve_idx) )+0.5);
					mean_p.y=floor(( double(mean_p.y)/double(curve_idx) )+0.5);
					for (int i=0;i<curve.size();i++)
						if (abs(mean_p.x-curve[i].x)<= mean_dist && abs(mean_p.y-curve[i].y) <= mean_dist)
							add_curve = false;
					
					if (add_curve) {
						if (inner_color_range > 0) {
							mean_inner_gray=0;
							//calc inner mean
							for (int i=0;i<curve.size();i++) {
								mean_inner_gray += mean_inner_grey_from_point(curve[i], mean_p, pic);
							}
							mean_inner_gray = floor(( double(mean_inner_gray)/double(curve.size()) )+0.5);
							
							if (mean_inner_gray_last > (mean_inner_gray + inner_color_range)) {
								mean_inner_gray_last = mean_inner_gray;
								all_curves.clear();
								all_curves.push_back(curve);
							} else if(mean_inner_gray_last <= (mean_inner_gray + inner_color_range) && mean_inner_gray_last >= (mean_inner_gray - inner_color_range)) {
								
								if(curve.size()>all_curves[0].size()){
									mean_inner_gray_last = mean_inner_gray;
									all_curves.clear();
									all_curves.push_back(curve);
								}
							}
						}
						else
							all_curves.push_back(curve);
					}
				}
			}
		}
	}
	
	return all_curves;
}
