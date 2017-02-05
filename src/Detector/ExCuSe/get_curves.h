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

	for(int i=0; i<IMG_SIZE; i++)
		for(int j=0; j<IMG_SIZE; j++)
			check[i][j]=0;


	for(int i=start_x; i<end_x; i++)
		for(int j=start_y; j<end_y; j++){

			
			

			if(edge->data[(edge->cols*(j))+(i)]==255 && !check[i][j]){
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
								if( edge->data[(edge->cols*(akt_pos.y+k2))+(akt_pos.x+k1)]==255){
								check[akt_pos.x+k1][akt_pos.y+k2]=1;
								
								mean_p.x+=akt_pos.x+k1;
								mean_p.y+=akt_pos.y+k2;
								curve.push_back(cv::Point(akt_pos.x+k1,akt_pos.y+k2));
								curve_idx++;
							}
							
					}
					akt_idx++;

				}





				if(curve_idx>0 && curve.size()>0){

				add_curve=true;
				mean_p.x=floor(( double(mean_p.x)/double(curve_idx) )+0.5);
				mean_p.y=floor(( double(mean_p.y)/double(curve_idx) )+0.5);
				for(int i=0;i<curve.size();i++) 
					if(  abs(mean_p.x-curve[i].x)<= mean_dist && abs(mean_p.y-curve[i].y) <= mean_dist)
						add_curve=false;



				if(add_curve) {
					
					if(inner_color_range>0){
						mean_inner_gray=0;
						//calc inner mean
						for(int i=0;i<curve.size();i++){

							if(pic->data[(pic->cols*(curve[i].y+1))+(curve[i].x)]!=0 || pic->data[(pic->cols*(curve[i].y-1))+(curve[i].x)]!=0 ) {
								if( sqrt( pow(double(curve[i].y-mean_p.y),2) + pow(double(curve[i].x-mean_p.x)+2,2)) < 
								   sqrt( pow(double(curve[i].y-mean_p.y),2) + pow(double(curve[i].x-mean_p.x)-2,2)) ) {

									mean_inner_gray+=(unsigned char)pic->data[(pic->cols*(curve[i].y))+(curve[i].x + 2)];
								} else {
									mean_inner_gray+=(unsigned char)pic->data[(pic->cols*(curve[i].y))+(curve[i].x - 2)];
								}

							} else if(pic->data[(pic->cols*(curve[i].y))+(curve[i].x+1)]!=0 || pic->data[(pic->cols*(curve[i].y))+(curve[i].x-1)]!=0 ) {
								if( sqrt( pow(double(curve[i].y-mean_p.y+2),2) + pow(double(curve[i].x-mean_p.x),2)) < 
									sqrt( pow(double(curve[i].y-mean_p.y-2),2) + pow(double(curve[i].x-mean_p.x),2)) ) {

									mean_inner_gray+=(unsigned char)pic->data[(pic->cols*(curve[i].y+2))+(curve[i].x)];
								} else {
									mean_inner_gray+=(unsigned char)pic->data[(pic->cols*(curve[i].y-2))+(curve[i].x)];
								}

							} else if(pic->data[(pic->cols*(curve[i].y+1))+(curve[i].x+1)]!=0 || pic->data[(pic->cols*(curve[i].y-1))+(curve[i].x-1)]!=0 ) {
								if( sqrt( pow(double(curve[i].y-mean_p.y-2),2) + pow(double(curve[i].x-mean_p.x+2),2)) < 
									sqrt( pow(double(curve[i].y-mean_p.y+2),2) + pow(double(curve[i].x-mean_p.x-2),2)) ) {

									mean_inner_gray+=(unsigned char)pic->data[(pic->cols*(curve[i].y-2))+(curve[i].x+2)];
								} else {
									mean_inner_gray+=(unsigned char)pic->data[(pic->cols*(curve[i].y+2))+(curve[i].x-2)];
								}

							} else if(pic->data[(pic->cols*(curve[i].y-1))+(curve[i].x+1)]!=0 || pic->data[(pic->cols*(curve[i].y+1))+(curve[i].x-1)]!=0 ) {
								if( sqrt( pow(double(curve[i].y-mean_p.y+2),2) + pow(double(curve[i].x-mean_p.x+2),2)) < 
								   sqrt( pow(double(curve[i].y-mean_p.y-2),2) + pow(double(curve[i].x-mean_p.x-2),2)) ) {

									mean_inner_gray+=(unsigned char)pic->data[(pic->cols*(curve[i].y+2))+(curve[i].x+2)];
								} else {
									mean_inner_gray+=(unsigned char)pic->data[(pic->cols*(curve[i].y-2))+(curve[i].x-2)];
								}
							}
						
						}
						mean_inner_gray=floor(( double(mean_inner_gray)/double(curve.size()) )+0.5);

						if(mean_inner_gray_last>(mean_inner_gray+inner_color_range)){
							mean_inner_gray_last=mean_inner_gray;
							all_curves.clear();
							all_curves.push_back(curve);
						}else if(mean_inner_gray_last<=(mean_inner_gray+inner_color_range) && mean_inner_gray_last>=(mean_inner_gray-inner_color_range)){

							if(curve.size()>all_curves[0].size()){
								mean_inner_gray_last=mean_inner_gray;
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

		


		return all_curves;
}
