//
//  QR.h
//  eyeFocus
//
//  Created by Oleg Geier on 08/03/17.
//
//

#ifndef QR_h
#define QR_h

#include <opencv2/imgproc/imgproc.hpp>

class QR {
public:
	/**
	 * QR-Decomposition. Solves overdetermined system of linear equations
	 * @param equations How many rows in A and b
	 * @param unknowns How many columns in A
	 * @param A Array with {equations} x {unknowns} elements. Row values, next row values, ...
	 * @param b Vector with {unknowns} elements
	 * @return Vector with {unknowns} elements
	 */
	static double* solve(int equations, int unknowns, double* A, double* b) {
		if (equations <= 0) {
			fputs("Error: QR decomposition needs equations to solve something.\n", stderr);
			return nullptr;
		}
		
		cv::Mat solved;
		cv::Mat internal_A = cv::Mat(equations, unknowns, CV_64FC1, A);
		cv::Mat internal_b = cv::Mat(equations, 1, CV_64FC1, b);
		cv::solve(internal_A, internal_b, solved, cv::DecompTypes::DECOMP_QR | cv::DecompTypes::DECOMP_NORMAL);
		
		int i = solved.rows;
		double* outX = new double[i];
		while (i--) outX[i] = solved.at<double>(i);
		return outX;
	}
	/*
	static double* solve_not(int equations, int unknowns, double* A, double* b) {
		if (equations <= 0) {
			fputs("Error: QR decomposition needs equations to solve something.\n", stderr);
			return nullptr;
		}
		
		cv::Mat R, Q;
		cv::Mat internal_A = cv::Mat(equations, unknowns, CV_64FC1, A);
		cv::Mat internal_b = cv::Mat(equations, 1, CV_64FC1, b);
		
		householder(internal_A, &R, &Q);
		
		cv::transpose(Q, Q);
		cv::Mat y = Q * internal_b;
		
		double* outX = new double[unknowns];
		int r = unknowns;
		while (r--) {
			outX[r] = y.at<double>(r, 0);
			for (int c = r+1; c < unknowns; c++)
				outX[r] -= R.at<double>(r, c) * outX[c];
			outX[r] /= R.at<double>(r, r);
		}
		
		internal_A.release();
		internal_b.release();
		y.release();
		R.release();
		Q.release();
		
		return outX;
	}
	
private:
	static cv::Mat matrix_minor(cv::Mat x, int d)
	{
		cv::Mat m = cv::Mat::zeros(x.rows, x.cols, CV_64FC1);
		for (int i = 0; i < d; i++)
			m.at<double>(i, i) = 1;
		for (int i = d; i < x.rows; i++)
			for (int j = d; j < x.cols; j++)
				m.at<double>(i, j) = x.at<double>(i, j);
		return m;
	}
	
	static void householder(cv::Mat m, cv::Mat *R, cv::Mat *Q) {
		cv::Mat z = m;
		cv::Mat I = cv::Mat::eye(m.rows, m.rows, CV_64FC1);
		cv::Mat tmp_Q = cv::Mat::eye(m.rows, m.rows, CV_64FC1);
		
		for (int k = 0; k < m.cols && k < m.rows - 1; k++) {
			z = matrix_minor(z, k);
			cv::Mat x = z.col(k);
			double a = cv::norm(x);
			if (m.at<double>(k, k) > 0)
				a = -a;
			
			cv::Mat e = cv::Mat::zeros(m.rows, 1, CV_64FC1);
			e.at<double>(k) = 1;
			
			e = x + e * a;
			e = e / cv::norm(e);
			
			cv::Mat eT;
			cv::transpose(e, eT);
			
			cv::Mat g = I + (e * eT) * -2;
			tmp_Q = g * tmp_Q;
			z = g * z;
		}
		z.release();
		I.release();
		
		*R = tmp_Q * m;
		cv::transpose(tmp_Q, *Q);
		tmp_Q.release();
	}*/
};
#endif /* QR_h */
