#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/core/core.hpp>

#define MAX_LINE 10000 

static void matlab_bwselect(cv::Mat *strong,cv::Mat *weak,cv::Mat *check){

	int pic_x=strong->cols;
	int pic_y=strong->rows;

	int lines[MAX_LINE];
	int lines_idx=0;


	int idx=0;

	char *p_strong, *p_weak, *p_check;
	for(int i=1;i<pic_y-1;i++){
		
		for(int j=1;j<pic_x-1;j++){

			if(strong->data[idx+j]!=0 && check->data[idx+j]==0){

				check->data[idx+j]=255;
				lines_idx=1;
				lines[0]=idx+j;

				
				int akt_idx=0;

				while(akt_idx<lines_idx && lines_idx<MAX_LINE-1){

					int akt_pos=lines[akt_idx];

					if(akt_pos-pic_x-1>=0 && akt_pos+pic_x+1<pic_x*pic_y){
					for(int k1=-1;k1<2;k1++)
						for(int k2=-1;k2<2;k2++){

		
							if(check->data[(akt_pos+(k1*pic_x))+k2]==0 && weak->data[(akt_pos+(k1*pic_x))+k2]!=0){

								check->data[(akt_pos+(k1*pic_x))+k2]=255;
								
								lines_idx++;
								lines[lines_idx-1]=(akt_pos+(k1*pic_x))+k2;
							}
							
					}
					}
					akt_idx++;

				}





			}

		}

	idx+=pic_x;
	}

}
