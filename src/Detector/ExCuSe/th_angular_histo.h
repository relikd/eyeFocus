
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/core/core.hpp>

#define DEF_SIZE 800 //max size of histogram -> max diagonal of image


/*
  Version 1.0, 08.06.2015, Copyright University of Tübingen.

  The Code is created based on the method from the paper:
  "ExCuSe: Robust Pupil Detection in Real-World Scenarios", W. Fuhl, T. C. Kübler, K. Sippel, W. Rosenstiel, E. Kasneci
  CAIP 2015 : Computer Analysis of Images and Patterns
 
  The code and the algorithm are for non-comercial use only.

*/


/*
Calculates the best position in a intesity histogram

input:
int *hist -> intesity histogram
int mini -> minimum intesity value threshold
int max_region_hole -> maximum invalid points in region
int min_region_size -> minimum region size
int real_hist_sz -> the real size of the histogram for position in image calculation

return:
int -> position in histogram
*/
static int calc_pos(int *hist, int mini, int max_region_hole, int min_region_size, int real_hist_sz){
	int pos=0;

	int mean_pos=0;
	int pos_hole=0;
	int count=0;
	int hole_size=0;
	bool region_start=false;



	for(int i=0; i<DEF_SIZE; i++){
		if(hist[i]>mini && !region_start){
			region_start=true;
			count++;
			mean_pos+=i;
		}else if(hist[i]>mini && region_start){
			count+=1+hole_size;
			mean_pos+=i+pos_hole;
			hole_size=0;
			pos_hole=0;
		}else if(hist[i]<=mini && region_start && hole_size<max_region_hole){
			hole_size++;
			pos_hole+=i;
		}else if(hist[i]<=mini && region_start && hole_size>=max_region_hole && count>=min_region_size){

			if(count<1) count=1;
			mean_pos=mean_pos/count;
			if( pow(double((real_hist_sz/2)-mean_pos),2) < pow(double((real_hist_sz/2)-pos),2) ) pos=mean_pos;

			pos_hole=0;
            hole_size=0;
            region_start=0;
            count=0;
            mean_pos=0;
		}else if(hist[i]<=mini && region_start && hole_size>=max_region_hole && count<min_region_size){
			pos_hole=0;
            hole_size=0;
            region_start=0;
            count=0;
            mean_pos=0;
		}

	}


	return pos;

}



/*
Calculates the coarse position of a possible pupil

input:
cv::Mat *pic -> gray scale image
cv::Mat *pic_th -> zero mat
int start_x -> start position in image x
int end_x -> start position in image y
int start_y -> end position in image x
int end_y -> end position in image y
double mean_dist -> line distance to mean for separation of curved and straight lines
int inner_color_range -> range in which gray values are considered equal

int th -> intesity threshold
double th_histo -> percentage factor for histogram threshold calculation
int max_region_hole -> maximum invalid points in region
int min_region_size -> minimum region size

return:
cv::Point -> position in image
*/
static cv::Point th_angular_histo(cv::Mat *pic, cv::Mat *pic_th, int start_x, int end_x, int start_y, int end_y, int th, double th_histo, int max_region_hole, int min_region_size){
	cv::Point pos(0,0);


	if(start_x<0) start_x=0;
	if(start_y<0) start_y=0;
	if(end_x>pic->cols) end_x=pic->cols;
	if(end_y>pic->rows) end_y=pic->rows;

	int max_l=0;
	int max_lb=0;
	int max_b=0;
	int max_br=0;

	int min_l, min_lb, min_b, min_br;
	int pos_l, pos_lb, pos_b, pos_br;


	int hist_l[DEF_SIZE];
	int hist_lb[DEF_SIZE];
	int hist_b[DEF_SIZE];
	int hist_br[DEF_SIZE];

	for(int i=0; i<DEF_SIZE; i++){
		hist_l[i]=0;
		hist_lb[i]=0;
		hist_b[i]=0;
		hist_br[i]=0;
	}

	int idx_lb=0;
	int idx_br=0;
	for(int i=start_x; i<end_x; i++)
		for(int j=start_y; j<end_y; j++){

			if(pic->data[(pic->cols*j)+i]<th){

				pic_th->data[(pic->cols*j)+i]=255;

				idx_lb=(pic->cols/2)+(i-(pic->cols/2))+(j);
				idx_br=(pic->cols/2)+(i-(pic->cols/2))+(pic->rows-j);

				if(j>=0 && j<DEF_SIZE && i>=0 && i<DEF_SIZE && idx_lb>=0 && idx_lb<DEF_SIZE && idx_br>=0 && idx_br<DEF_SIZE){

				if(++hist_l[j]>max_l) max_l=hist_l[j];

				if(++hist_b[i]>max_b) max_b=hist_b[i];


				if(++hist_lb[idx_lb]>max_lb) max_lb=hist_lb[idx_lb];

				if(++hist_br[idx_br]>max_br) max_br=hist_br[idx_br];
			
				}
			}

		}

	


	min_l=max_l-floor(max_l*th_histo);
	min_lb=max_lb-floor(max_lb*th_histo);
	min_b=max_b-floor(max_b*th_histo);
	min_br=max_br-floor(max_br*th_histo);

	pos_l=calc_pos(hist_l, min_l, max_region_hole, min_region_size, pic->rows);
	pos_lb=calc_pos(hist_lb, min_lb, max_region_hole, min_region_size, pic->cols+pic->rows);
	pos_b=calc_pos(hist_b, min_b, max_region_hole, min_region_size, pic->cols);
	pos_br=calc_pos(hist_br, min_br, max_region_hole, min_region_size, pic->cols+pic->rows);


	/*
std::cout<<"min_l: "<<min_l<<" min_lb: "<<min_lb<<std::endl;	
std::cout<<"min_b: "<<min_b<<" min_br: "<<min_br<<std::endl;	

std::cout<<"l: "<<pos_l<<"    lb: "<<pos_lb<<std::endl;
std::cout<<"b: "<<pos_b<<"    br: "<<pos_br<<std::endl;
*/
	
	if(pos_l>0 && pos_lb>0 && pos_b>0 && pos_br>0){
		pos.x=floor( ((pos_b+ (floor( (((pos_br+pic->rows)-pos_lb)/2) +0.5) +pos_lb-pic->rows) )/2)+0.5);
		pos.y=floor( ((pos_l+ (pic->rows - floor( (((pos_br+pic->rows)-pos_lb)/2) +0.5)) )/2)+0.5);
	}else if(pos_l>0 && pos_b>0){
		pos.x=pos_b;
		pos.y=pos_l;
	}else if(pos_lb>0 && pos_br>0){
		pos.x=floor( (((pos_br+pic->rows)-pos_lb)/2) +0.5) +pos_lb-pic->rows ;
		pos.y=pic->rows - floor( (((pos_br+pic->rows)-pos_lb)/2) +0.5);
	}

	if(pos.x<0) pos.x=0;
	if(pos.y<0) pos.y=0;
	if(pos.x>=pic->cols) pos.x=0;
	if(pos.y>=pic->rows) pos.y=0;

	
	/*
	std::cout<<pos.x<<";"<<pos.y<<std::endl;
	imshow("th",*pic_th);
	cv::ellipse(*pic, cv::RotatedRect(pos, cv::Size2f(5,5),0), CV_RGB(255,255,255));
	imshow("angular",*pic);
	*/

	return pos;

}
