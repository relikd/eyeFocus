#include <stdio.h>
#include <stdlib.h>
#include <cstring>
#include <functional>
#include <cmath>
#include <vector>

#define NumOfCols 10
const size_t skipFirstXValues = 0;

//  ---------------------------------------------------------------
// |
// |  Helper
// |
//  ---------------------------------------------------------------

struct MinMaxAvg {
	float min[NumOfCols], max[NumOfCols], avg[NumOfCols];
	size_t count = 0;
	
	MinMaxAvg() {
		for (int i = 0; i < NumOfCols; i++) {
			min[i] = 99999; max[i] = 0; avg[i] = 0;
		}
	}
	void update(const float* vector) {
		for (int i = 0; i < NumOfCols; i++) {
			avg[i] += vector[i];
			if (min[i] > vector[i]) min[i] = vector[i];
			if (max[i] < vector[i]) max[i] = vector[i];
		}
		++count;
	}
	size_t calculateAverage() {
		size_t countBeforeReset = count;
		if (count > 0) {
			for (int i = 0; i < NumOfCols; i++)
				avg[i] /= count;
			count = 0;
		}
		return countBeforeReset;
	}
};

FILE* openFile(const char* path, bool write = false) {
	FILE* file;
#ifdef _WIN32
	fopen_s(&file, path, (write?"w":"r"));
#else
	file = fopen(path, (write?"w":"r"));
#endif
	return file;
}

size_t scanFile(FILE* file, MinMaxAvg* mma, float* data) {
	size_t skip = skipFirstXValues;
	size_t i = 0;
	while (true) {
		float val[NumOfCols];
		int n = fscanf(file, "%f,%f,%f,%f,%f,%f,%f,%f,%f,%f\n", &val[0],&val[1],&val[2],&val[3],&val[4],&val[5],&val[6],&val[7],&val[8],&val[9]);
		if (n == EOF) {
			break;
		} else if (n == 0) { // damaged data
			fscanf(file, "%*[^\n]\n", NULL); // skip line
			continue;
		} else if (skip) {
			--skip; // skip first values because of Kalman filter
			continue;
		}
		// actual processing
		mma->update(val);
		memcpy(&data[i*NumOfCols], val, sizeof(float)*NumOfCols);
		i++;
	}
	return mma->calculateAverage();
}

void printVarCovMat(float* var, float cov, size_t count) {
	printf("%f , 0 , 0 , 0\n",  var[0] / count);
	printf("0 , %f , 0 , %f\n", var[1] / count,    cov / count);
	printf("0 , 0 , %f , 0\n",  var[2] / count);
	printf("0 , %f , 0 , %f\n",    cov / count, var[3] / count);
	printf("\n");
}

//  ---------------------------------------------------------------
// |
// |  Main
// |
//  ---------------------------------------------------------------

int main( int argc, const char** argv ) {
	if (argc < 2) {
		fputs("Missing argument value. Path to report file[s] expected.\n\n", stderr);
		return EXIT_SUCCESS;
	}
	std::vector<float> frameDiff[4];
	
	float totalVar[4] = {0,0,0,0};
	float totalCov = 0;
	size_t totalCount = 0;
	for (int i=1; i<argc; i++) {
		FILE* file = openFile(argv[i]);
		if (!file) {
			printf("Couldn't open file '%s'.\n", argv[i]);
			continue;
		}
		
		fscanf(file, "%*[^\n]\n", NULL); // skip header
		
		MinMaxAvg local;
		float* data = new float[2000 * NumOfCols];
		size_t localCount = scanFile(file, &local, data);
		totalCount += localCount;
		
		float pre[4] = {
			data[(-1+localCount)*NumOfCols+0],
			data[(-1+localCount)*NumOfCols+1],
			data[(-1+localCount)*NumOfCols+2],
			data[(-1+localCount)*NumOfCols+3]};
		float maxdiff[4] = {0,0,0,0};
		
		float localVar[4] = {0,0,0,0};
		float localCov = 0;
		size_t c = localCount;
		while (c--) {
			// calc variance and covariance
			for (int u = 0; u < 4; u++) {
				float meanDiff = (data[c * NumOfCols + u] - local.avg[u]);
				localVar[u] += meanDiff * meanDiff;
				totalVar[u] += meanDiff * meanDiff;
				
				float tmpDiff = fabs(pre[u] - data[c * NumOfCols + u]);
				frameDiff[u].push_back(tmpDiff);
				if (tmpDiff > maxdiff[u]) {
					maxdiff[u] = tmpDiff;
				}
				pre[u] = data[c * NumOfCols + u];
			}
			float tCov = (data[c * NumOfCols + 1] - local.avg[1]) * (data[c * NumOfCols + 3] - local.avg[3]);
			localCov += tCov;
			totalCov += tCov;
		}
		printf("%s:\n\n", argv[i]);
		printf("   [ Max-change (frame to frame) ]\n%f, %f, %f, %f\n\n", maxdiff[0],maxdiff[1],maxdiff[2],maxdiff[3]);
		printf("   [ Variance-Covariance-Matrix ]\n");
		printVarCovMat(localVar, localCov, localCount-1);
		printf("---------------------------\n\n");
		fclose(file);
	}
	
	printf("   [ Combined ]\n");
	printVarCovMat(totalVar, totalCov, totalCount-1);
	
	
	// 99%-Quantil
	FILE* fDiff = openFile("./frameDifference.txt", true);
	if (fDiff) {
		fprintf(fDiff, "   Xl   ,   Yl   ,   Xr   ,   Yr   \n");
		size_t sampleCount = frameDiff[0].size();
		for (int u = 0; u < 4; u++)
			std::sort(frameDiff[u].begin(), frameDiff[u].end());
		for (size_t i = 0; i < sampleCount; i++)
			fprintf(fDiff, "%1.6f,%1.6f,%1.6f,%1.6f\n", frameDiff[0][i],frameDiff[1][i],frameDiff[2][i],frameDiff[3][i]);
		printf("Samples: %lu\n", sampleCount);
		size_t p99 = sampleCount * 0.99; // since index it will be always +1
		printf("99%%-Quantil: %1.6f , %1.6f , %1.6f , %1.6f\n", frameDiff[0][p99],frameDiff[1][p99],frameDiff[2][p99],frameDiff[3][p99]);
		fclose(fDiff);
	}
	
	printf("\n");
	return EXIT_SUCCESS;
}

