
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/core/core.hpp>

/*
  Version 1.0, 08.06.2015, Copyright University of Tübingen.

  The Code is created based on the method from the paper:
  "ExCuSe: Robust Pupil Detection in Real-World Scenarios", W. Fuhl, T. C. Kübler, K. Sippel, W. Rosenstiel, E. Kasneci
  CAIP 2015 : Computer Analysis of Images and Patterns
 
  The code and the algorithm are for non-comercial use only.

*/


/*
Optimizes input position based on pixel neighborhood

input:
cv::Mat *pic -> gray scale image
double area -> percentage of window size
cv::Point *pos -> starting and result position

*/
static void optimize_pos(cv::Mat *pic, double area, cv::Point *pos){
	

	int start_x=pos->x - (area*pic->cols);
	int end_x=pos->x + (area*pic->cols);
	int start_y=pos->y - (area*pic->rows);
	int end_y=pos->y + (area*pic->rows);


	int val;
	int min_akt;
	int min_val=1000000;

	int pos_x=0;
	int pos_y=0;
	int pos_count=0;
	
	int reg_size=sqrt(sqrt(pow(double(area*pic->cols*2),2) + pow(double(area*pic->rows*2),2)));



	if(start_x<reg_size) start_x=reg_size;
	if(start_y<reg_size) start_y=reg_size;
	if(end_x>pic->cols) end_x=pic->cols-(reg_size+1);
	if(end_y>pic->rows) end_y=pic->rows-(reg_size+1);



	for(int i=start_x; i<end_x; i++)
		for(int j=start_y; j<end_y; j++){

			min_akt=0;

			for(int k1=-reg_size; k1<reg_size; k1++)
				for(int k2=-reg_size; k2<reg_size; k2++){

					if(i+k1>0 && i+k1<pic->cols && j+k2>0 && j+k2<pic->rows){
					val=(pic->data[(pic->cols*j)+(i)]-pic->data[(pic->cols*(j+k2))+(i+k1)]);
					if(val>0) min_akt+=val;
					}

				}

			if(min_akt==min_val){
				pos_x+=i;
				pos_y+=j;
				pos_count++;
			}

			if(min_akt<min_val){
				min_val=min_akt;
				pos_x=i;
				pos_y=j;
				pos_count=1;
			}

		}


	if(pos_count>0){
		pos->x=pos_x/pos_count;
		pos->y=pos_y/pos_count;
	}
}
