//
//  helper.h
//  eyeFocus
//
//  Created by Oleg Geier on 19/01/17.
//
//

#ifndef helper_h
#define helper_h

#include <iostream>
#include <chrono>

class Timer {
public:
	Timer() : beg_(clock_::now()) {}
	void reset() { beg_ = clock_::now(); }
	double elapsed() const {
		return std::chrono::duration_cast<second_>
		(clock_::now() - beg_).count(); }
	
private:
	typedef std::chrono::high_resolution_clock clock_;
	typedef std::chrono::duration<double, std::ratio<1> > second_;
	std::chrono::time_point<clock_> beg_;
};

#define CompareTime(_times_,__1__,__2__) \
Timer performanceTimer;\
double timeFirst, timeSecond;\
size_t repeatTimes = _times_;\
performanceTimer.reset();\
while (repeatTimes--) {__1__;}\
timeFirst = performanceTimer.elapsed();\
repeatTimes = _times_;\
performanceTimer.reset();\
while (repeatTimes--) {__2__;}\
timeSecond = performanceTimer.elapsed();\
printf("-> Algorithm is %1.2fx faster (%1.4f, %1.4f)\n", timeFirst / timeSecond, timeFirst, timeSecond);

template <class T>
void printMat(cv::Mat &ref) {
	printf("\n{");
	for (int o = 0; o < ref.rows; o++) {
		T *tempo = ref.ptr<T>(o);
		printf("[");
		for (int i = 0; i < ref.cols; i++) {
			//printf("%d, ", tempo[i] );
			std::cout << tempo[i] << ", ";
		}
		printf("]\n");
	}
	printf("}\n");
}


#endif /* helper_h */
