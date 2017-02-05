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
 Performs morphologic operations on the edge image
 
 input:
 cv::Mat *edge -> edge image
 int start_xx -> start position in image x
 int end_xx -> start position in image y
 int start_yy -> end position in image x
 int end_yy -> end position in image y
 
 
 */
static void remove_points_with_low_angle(cv::Mat edge, int start_xx, int end_xx, int start_yy, int end_yy){
	
	
	
	int start_x = start_xx+5;
	int end_x = end_xx-5;
	int start_y = start_yy+5;
	int end_y = end_yy-5;
	
	
	if(start_x<5) start_x = 5;
	if(end_x>edge.cols-5) end_x = edge.cols-5;
	if(start_y<5) start_y = 5;
	if(end_y>edge.rows-5) end_y = edge.rows-5;
	
	
	//cv::imshow("start",*edge);
	
	
	
	for(int j = start_y; j<end_y; j++)
		for(int i = start_x; i<end_x; i++){
			
			
			
			if(edge.at<uchar>(j, i)){
				uchar box[8];
				
				box[0] = edge.at<uchar>(j-1, i-1);
				box[1] = edge.at<uchar>(j-1, i);
				box[2] = edge.at<uchar>(j-1, i+1);
				box[3] = edge.at<uchar>(j, i+1);
				box[4] = edge.at<uchar>(j+1, i+1);
				box[5] = edge.at<uchar>(j+1, i);
				box[6] = edge.at<uchar>(j+1, i-1);
				box[7] = edge.at<uchar>(j, i-1);
				
				bool valid = false;
				
				for(int k = 0;k<8 && !valid;k++)
					//if( box[k] && (box[(k+3)%8] || box[(k+4)%8] || box[(k+5)%8]) ) valid = true;
					if( box[k] && (box[(k+2)%8] || box[(k+3)%8] || box[(k+4)%8] || box[(k+5)%8] || box[(k+6)%8]) ) valid = true;
				
				if(!valid) edge.at<uchar>(j, i) = 0;
				
				
			}
		}
	
	
	
	//cv::imshow("angle",*edge);
	
	
	for(int j = start_y; j<end_y; j++)
		for(int i = start_x; i<end_x; i++){
			uchar box[9];
			
			box[4] = edge.at<uchar>(j, i);
			
			if(box[4]){
				box[1] = edge.at<uchar>(j-1, i);
				box[3] = edge.at<uchar>(j, i-1);
				box[5] = edge.at<uchar>(j, i+1);
				box[7] = edge.at<uchar>(j+1, i);
				
				
				if((box[5] && box[7])) edge.at<uchar>(j, i) = 0;
				if((box[5] && box[1])) edge.at<uchar>(j, i) = 0;
				if((box[3] && box[7])) edge.at<uchar>(j, i) = 0;
				if((box[3] && box[1])) edge.at<uchar>(j, i) = 0;
				
				
				//if( (box[1] && box[5]) || (box[1] && box[3]) || (box[3] && box[7]) || (box[5] && box[7]) )
				//		edge.at<uchar>(j, i) = 0;
			}
		}
	
	//cv::imshow("morph1",*edge);
	
	for(int j = start_y; j<end_y; j++)
		for(int i = start_x; i<end_x; i++){
			uchar box[17];
			
			box[4] = edge.at<uchar>(j, i);
			
			if(box[4]){
				box[0] = edge.at<uchar>(j-1, i-1);
				box[1] = edge.at<uchar>(j-1, i);
				box[2] = edge.at<uchar>(j-1, i+1);
				
				box[3] = edge.at<uchar>(j, i-1);
				box[5] = edge.at<uchar>(j, i+1);
				
				box[6] = edge.at<uchar>(j+1, i-1);
				box[7] = edge.at<uchar>(j+1, i);
				box[8] = edge.at<uchar>(j+1, i+1);
				
				//external
				box[9] = edge.at<uchar>(j, i+2);
				box[10] = edge.at<uchar>(j+2, i);
				
				
				box[11] = edge.at<uchar>(j, i+3);
				box[12] = edge.at<uchar>(j-1, i+2);
				box[13] = edge.at<uchar>(j+1, i+2);
				
				
				box[14] = edge.at<uchar>(j+3, i);
				box[15] = edge.at<uchar>(j+2, i-1);
				box[16] = edge.at<uchar>(j+2, i+1);
				
				
				
				if( (box[10] && !box[7]) && (box[8] || box[6]) ){
					edge.at<uchar>(j+1, i-1) = 0;
					edge.at<uchar>(j+1, i+1) = 0;
					edge.at<uchar>(j+1, i) = 255;
				}
				
				
				if( (box[14] && !box[7] && !box[10]) && ( (box[8] || box[6]) && (box[16] || box[15]) ) ){
					edge.at<uchar>(j+1, i+1) = 0;
					edge.at<uchar>(j+1, i-1) = 0;
					edge.at<uchar>(j+2, i+1) = 0;
					edge.at<uchar>(j+2, i-1) = 0;
					edge.at<uchar>(j+1, i) = 255;
					edge.at<uchar>(j+2, i) = 255;
				}
				
				
				
				if( (box[9] && !box[5]) && (box[8] || box[2]) ){
					edge.at<uchar>(j+1, i+1) = 0;
					edge.at<uchar>(j-1, i+1) = 0;
					edge.at<uchar>(j, i+1) = 255;
				}
				
				
				if( (box[11] && !box[5] && !box[9]) && ( (box[8] || box[2]) && (box[13] || box[12]) ) ){
					edge.at<uchar>(j+1, i+1) = 0;
					edge.at<uchar>(j-1, i+1) = 0;
					edge.at<uchar>(j+1, i+2) = 0;
					edge.at<uchar>(j-1, i+2) = 0;
					edge.at<uchar>(j, i+1) = 255;
					edge.at<uchar>(j, i+2) = 255;
				}
				
				
			}
		}
	
	//cv::imshow("morph2",*edge);
	
	
	for(int j = start_y; j<end_y; j++)
		for(int i = start_x; i<end_x; i++){
			
			uchar box[33];
			
			box[4] = edge.at<uchar>(j, i);
			
			if(box[4]){
				box[0] = edge.at<uchar>(j-1, i-1);
				box[1] = edge.at<uchar>(j-1, i);
				box[2] = edge.at<uchar>(j-1, i+1);
				
				box[3] = edge.at<uchar>(j, i-1);
				box[5] = edge.at<uchar>(j, i+1);
				
				box[6] = edge.at<uchar>(j+1, i-1);
				box[7] = edge.at<uchar>(j+1, i);
				box[8] = edge.at<uchar>(j+1, i+1);
				
				box[9] = edge.at<uchar>(j-1, i+2);
				box[10] = edge.at<uchar>(j-1, i-2);
				box[11] = edge.at<uchar>(j+1, i+2);
				box[12] = edge.at<uchar>(j+1, i-2);
				
				
				box[13] = edge.at<uchar>(j-2, i-1);
				box[14] = edge.at<uchar>(j-2, i+1);
				box[15] = edge.at<uchar>(j+2, i-1);
				box[16] = edge.at<uchar>(j+2, i+1);
				
				box[17] = edge.at<uchar>(j-3, i-1);
				box[18] = edge.at<uchar>(j-3, i+1);
				box[19] = edge.at<uchar>(j+3, i-1);
				box[20] = edge.at<uchar>(j+3, i+1);
				
				box[21] = edge.at<uchar>(j+1, i+3);
				box[22] = edge.at<uchar>(j+1, i-3);
				box[23] = edge.at<uchar>(j-1, i+3);
				box[24] = edge.at<uchar>(j-1, i-3);
				
				box[25] = edge.at<uchar>(j-2, i-2);
				box[26] = edge.at<uchar>(j+2, i+2);
				box[27] = edge.at<uchar>(j-2, i+2);
				box[28] = edge.at<uchar>(j+2, i-2);
				
				box[29] = edge.at<uchar>(j-3, i-3);
				box[30] = edge.at<uchar>(j+3, i+3);
				box[31] = edge.at<uchar>(j-3, i+3);
				box[32] = edge.at<uchar>(j+3, i-3);
				
				
				
				if( box[7] && box[2] && box[9] )
					edge.at<uchar>(j, i) = 0;
				if( box[7] && box[0] && box[10] )
					edge.at<uchar>(j, i) = 0;
				if( box[1] && box[8] && box[11] )
					edge.at<uchar>(j, i) = 0;
				if( box[1] && box[6] && box[12] )
					edge.at<uchar>(j, i) = 0;
				
				if( box[0] && box[13] && box[17] && box[8] && box[11] && box[21] )
					edge.at<uchar>(j, i) = 0;
				if( box[2] && box[14] && box[18] && box[6] && box[12] && box[22] )
					edge.at<uchar>(j, i) = 0;
				if( box[6] && box[15] && box[19] && box[2] && box[9] && box[23] )
					edge.at<uchar>(j, i) = 0;
				if( box[8] && box[16] && box[20] && box[0] && box[10] && box[24] )
					edge.at<uchar>(j, i) = 0;
				
				if( box[0] && box[25] && box[29] && box[2] && box[27] && box[31] )
					edge.at<uchar>(j, i) = 0;
				if( box[0] && box[25] && box[29] && box[6] && box[28] && box[32] )
					edge.at<uchar>(j, i) = 0;
				if( box[8] && box[26] && box[30] && box[2] && box[27] && box[31] )
					edge.at<uchar>(j, i) = 0;
				if( box[8] && box[26] && box[30] && box[6] && box[28] && box[32] )
					edge.at<uchar>(j, i) = 0;
				
			}
			
		}
	
	
	//cv::imshow("morph3",*edge);
}
