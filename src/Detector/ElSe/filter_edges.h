#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/core/core.hpp>

/*
  Version 1.0, 17.12.2015, Copyright University of Tübingen.

  The Code is created based on the method from the paper:
  "ElSe: Ellipse Selection for Robust Pupil Detection in Real-World Environments", W. Fuhl, T. C. Santini, T. C. Kübler, E. Kasneci
  ETRA 2016 : Eye Tracking Research and Application 2016
 
  The code and the algorithm are for non-comercial use only.

*/

namespace ELSE{


static void filter_edges(cv::Mat *edge, int start_xx, int end_xx, int start_yy, int end_yy){



		int start_x=start_xx+5;
		int end_x=end_xx-5;
		int start_y=start_yy+5;
		int end_y=end_yy-5;


		if(start_x<5) start_x=5;
		if(end_x>edge->cols-5) end_x=edge->cols-5;
		if(start_y<5) start_y=5;
		if(end_y>edge->rows-5) end_y=edge->rows-5;




		for(int j=start_y; j<end_y; j++)
		for(int i=start_x; i<end_x; i++){
			int box[9];

			box[4]=(int)edge->data[(edge->cols*(j))+(i)];

			if(box[4]){
				box[1]=(int)edge->data[(edge->cols*(j-1))+(i)];
				box[3]=(int)edge->data[(edge->cols*(j))+(i-1)];
				box[5]=(int)edge->data[(edge->cols*(j))+(i+1)];
				box[7]=(int)edge->data[(edge->cols*(j+1))+(i)];


				if((box[5] && box[7])) edge->data[(edge->cols*(j))+(i)]=0;
				if((box[5] && box[1])) edge->data[(edge->cols*(j))+(i)]=0;
				if((box[3] && box[7])) edge->data[(edge->cols*(j))+(i)]=0;
				if((box[3] && box[1])) edge->data[(edge->cols*(j))+(i)]=0;

			}
		}



		//too many neigbours
		for(int j=start_y; j<end_y; j++)
		for(int i=start_x; i<end_x; i++){
			int neig=0;

			for(int k1=-1;k1<2;k1++)
				for(int k2=-1;k2<2;k2++){

					if(edge->data[(edge->cols*(j+k1))+(i+k2)]>0)
						neig++;
				}

			if(neig>3)
				edge->data[(edge->cols*(j))+(i)]=0;

		}






		for(int j=start_y; j<end_y; j++)
		for(int i=start_x; i<end_x; i++){
			int box[17];

			box[4]=(int)edge->data[(edge->cols*(j))+(i)];

			if(box[4]){
				box[0]=(int)edge->data[(edge->cols*(j-1))+(i-1)];
				box[1]=(int)edge->data[(edge->cols*(j-1))+(i)];
				box[2]=(int)edge->data[(edge->cols*(j-1))+(i+1)];

				box[3]=(int)edge->data[(edge->cols*(j))+(i-1)];
				box[5]=(int)edge->data[(edge->cols*(j))+(i+1)];

				box[6]=(int)edge->data[(edge->cols*(j+1))+(i-1)];
				box[7]=(int)edge->data[(edge->cols*(j+1))+(i)];
				box[8]=(int)edge->data[(edge->cols*(j+1))+(i+1)];

				//external
				box[9]=(int)edge->data[(edge->cols*(j))+(i+2)];
				box[10]=(int)edge->data[(edge->cols*(j+2))+(i)];


				box[11]=(int)edge->data[(edge->cols*(j))+(i+3)];
				box[12]=(int)edge->data[(edge->cols*(j-1))+(i+2)];
				box[13]=(int)edge->data[(edge->cols*(j+1))+(i+2)];

				
				box[14]=(int)edge->data[(edge->cols*(j+3))+(i)];
				box[15]=(int)edge->data[(edge->cols*(j+2))+(i-1)];
				box[16]=(int)edge->data[(edge->cols*(j+2))+(i+1)];



				if( (box[10] && !box[7]) && (box[8] || box[6]) ){
						edge->data[(edge->cols*(j+1))+(i-1)]=0;
						edge->data[(edge->cols*(j+1))+(i+1)]=0;
						edge->data[(edge->cols*(j+1))+(i)]=255;
				}

				
				if( (box[14] && !box[7] && !box[10]) && ( (box[8] || box[6]) && (box[16] || box[15]) ) ){
						edge->data[(edge->cols*(j+1))+(i+1)]=0;
						edge->data[(edge->cols*(j+1))+(i-1)]=0;
						edge->data[(edge->cols*(j+2))+(i+1)]=0;
						edge->data[(edge->cols*(j+2))+(i-1)]=0;
						edge->data[(edge->cols*(j+1))+(i)]=255;
						edge->data[(edge->cols*(j+2))+(i)]=255;
				}
				


				if( (box[9] && !box[5]) && (box[8] || box[2]) ){
						edge->data[(edge->cols*(j+1))+(i+1)]=0;
						edge->data[(edge->cols*(j-1))+(i+1)]=0;
						edge->data[(edge->cols*(j))+(i+1)]=255;
				}

				
				if( (box[11] && !box[5] && !box[9]) && ( (box[8] || box[2]) && (box[13] || box[12]) ) ){
						edge->data[(edge->cols*(j+1))+(i+1)]=0;
						edge->data[(edge->cols*(j-1))+(i+1)]=0;
						edge->data[(edge->cols*(j+1))+(i+2)]=0;
						edge->data[(edge->cols*(j-1))+(i+2)]=0;
						edge->data[(edge->cols*(j))+(i+1)]=255;
						edge->data[(edge->cols*(j))+(i+2)]=255;
				}
				





			}
		}

		
		
		for(int j=start_y; j<end_y; j++)
		for(int i=start_x; i<end_x; i++){

			int box[33];

			box[4]=(int)edge->data[(edge->cols*(j))+(i)];

			if(box[4]){
				box[0]=(int)edge->data[(edge->cols*(j-1))+(i-1)];
				box[1]=(int)edge->data[(edge->cols*(j-1))+(i)];
				box[2]=(int)edge->data[(edge->cols*(j-1))+(i+1)];

				box[3]=(int)edge->data[(edge->cols*(j))+(i-1)];
				box[5]=(int)edge->data[(edge->cols*(j))+(i+1)];

				box[6]=(int)edge->data[(edge->cols*(j+1))+(i-1)];
				box[7]=(int)edge->data[(edge->cols*(j+1))+(i)];
				box[8]=(int)edge->data[(edge->cols*(j+1))+(i+1)];

				box[9]=(int)edge->data[(edge->cols*(j-1))+(i+2)];
				box[10]=(int)edge->data[(edge->cols*(j-1))+(i-2)];
				box[11]=(int)edge->data[(edge->cols*(j+1))+(i+2)];
				box[12]=(int)edge->data[(edge->cols*(j+1))+(i-2)];


				box[13]=(int)edge->data[(edge->cols*(j-2))+(i-1)];
				box[14]=(int)edge->data[(edge->cols*(j-2))+(i+1)];
				box[15]=(int)edge->data[(edge->cols*(j+2))+(i-1)];
				box[16]=(int)edge->data[(edge->cols*(j+2))+(i+1)];

				box[17]=(int)edge->data[(edge->cols*(j-3))+(i-1)];
				box[18]=(int)edge->data[(edge->cols*(j-3))+(i+1)];
				box[19]=(int)edge->data[(edge->cols*(j+3))+(i-1)];
				box[20]=(int)edge->data[(edge->cols*(j+3))+(i+1)];

				box[21]=(int)edge->data[(edge->cols*(j+1))+(i+3)];
				box[22]=(int)edge->data[(edge->cols*(j+1))+(i-3)];
				box[23]=(int)edge->data[(edge->cols*(j-1))+(i+3)];
				box[24]=(int)edge->data[(edge->cols*(j-1))+(i-3)];

				box[25]=(int)edge->data[(edge->cols*(j-2))+(i-2)];
				box[26]=(int)edge->data[(edge->cols*(j+2))+(i+2)];
				box[27]=(int)edge->data[(edge->cols*(j-2))+(i+2)];
				box[28]=(int)edge->data[(edge->cols*(j+2))+(i-2)];

				box[29]=(int)edge->data[(edge->cols*(j-3))+(i-3)];
				box[30]=(int)edge->data[(edge->cols*(j+3))+(i+3)];
				box[31]=(int)edge->data[(edge->cols*(j-3))+(i+3)];
				box[32]=(int)edge->data[(edge->cols*(j+3))+(i-3)];




				
				if( box[7] && box[2] && box[9] )
						edge->data[(edge->cols*(j))+(i)]=0;
				if( box[7] && box[0] && box[10] )
						edge->data[(edge->cols*(j))+(i)]=0;
				if( box[1] && box[8] && box[11] )
						edge->data[(edge->cols*(j))+(i)]=0;
				if( box[1] && box[6] && box[12] )
						edge->data[(edge->cols*(j))+(i)]=0;
				
				
				if( box[0] && box[13] && box[17] && box[8] && box[11] && box[21] )
						edge->data[(edge->cols*(j))+(i)]=0;
				if( box[2] && box[14] && box[18] && box[6] && box[12] && box[22] )
						edge->data[(edge->cols*(j))+(i)]=0;
				if( box[6] && box[15] && box[19] && box[2] && box[9] && box[23] )
						edge->data[(edge->cols*(j))+(i)]=0;
				if( box[8] && box[16] && box[20] && box[0] && box[10] && box[24] )
						edge->data[(edge->cols*(j))+(i)]=0;




				if( box[0] && box[25] && box[2] && box[27] )
						edge->data[(edge->cols*(j))+(i)]=0;
				if( box[0] && box[25] && box[6] && box[28] )
						edge->data[(edge->cols*(j))+(i)]=0;
				if( box[8] && box[26] && box[2] && box[27] )
						edge->data[(edge->cols*(j))+(i)]=0;
				if( box[8] && box[26] && box[6] && box[28] )
						edge->data[(edge->cols*(j))+(i)]=0;




				
				int box2[18];
				box2[1]=(int)edge->data[(edge->cols*(j))+(i-1)];

				box2[2]=(int)edge->data[(edge->cols*(j-1))+(i-2)];
				box2[3]=(int)edge->data[(edge->cols*(j-2))+(i-3)];

				box2[4]=(int)edge->data[(edge->cols*(j-1))+(i+1)];
				box2[5]=(int)edge->data[(edge->cols*(j-2))+(i+2)];

				box2[6]=(int)edge->data[(edge->cols*(j+1))+(i-2)];
				box2[7]=(int)edge->data[(edge->cols*(j+2))+(i-3)];

				box2[8]=(int)edge->data[(edge->cols*(j+1))+(i+1)];
				box2[9]=(int)edge->data[(edge->cols*(j+2))+(i+2)];



				box2[10]=(int)edge->data[(edge->cols*(j+1))+(i)];

				box2[15]=(int)edge->data[(edge->cols*(j-1))+(i-1)];
				box2[16]=(int)edge->data[(edge->cols*(j-2))+(i-2)];

				
				box2[11]=(int)edge->data[(edge->cols*(j+2))+(i+1)];
				box2[12]=(int)edge->data[(edge->cols*(j+3))+(i+2)];

				box2[13]=(int)edge->data[(edge->cols*(j+2))+(i-1)];
				box2[14]=(int)edge->data[(edge->cols*(j+3))+(i-2)];


				if( box2[1] && box2[2] && box2[3] && box2[4] && box2[5] )
						edge->data[(edge->cols*(j))+(i)]=0;
				if( box2[1] && box2[6] && box2[7] && box2[8] && box2[9] )
						edge->data[(edge->cols*(j))+(i)]=0;
				if( box2[10] && box2[11] && box2[12] && box2[4] && box2[5] )
						edge->data[(edge->cols*(j))+(i)]=0;
				if( box2[10] && box2[13] && box2[14] && box2[15] && box2[16] )
						edge->data[(edge->cols*(j))+(i)]=0;



			}

		}


}


}
