#include <stdio.h>
#include <stdlib.h>
#include <cstring>
#include <functional>
#include <cmath>
#include <vector>

#define NumOfCols 10+1

const char* file_error = "meanError.csv";
const char* file_report = "report.txt";

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

struct Difference {
	float toMin[NumOfCols], toMax[NumOfCols], total[NumOfCols];
	
	Difference() {
		for (int i = 0; i < NumOfCols; i++) {
			toMin[i] = 0; toMax[i] = 0; total[i] = 0;
		}
	}
	Difference(const MinMaxAvg &mma) {
		for (int i = 0; i < NumOfCols; i++) {
			toMin[i] = mma.avg[i] - mma.min[i];
			toMax[i] = mma.max[i] - mma.avg[i];
			total[i] = mma.max[i] - mma.min[i];
		}
	}
	void update(const Difference &other) {
		for (int i = 0; i < NumOfCols; i++) {
			if (toMin[i] < other.toMin[i]) toMin[i] = other.toMin[i];
			if (toMax[i] < other.toMax[i]) toMax[i] = other.toMax[i];
			if (total[i] < other.total[i]) total[i] = other.total[i];
		}
	}
};

FILE* openFile(const char* path, const char* name, bool write = false) {
	char fullPath[1024];
	snprintf(fullPath, 1024*sizeof(char), "%s/%s", path, name);
	FILE* file;
#ifdef _WIN32
	fopen_s(&file, fullPath, (write?"w":"r"));
#else
	file = fopen(fullPath, (write?"w":"r"));
#endif
	return file;
}



//  ---------------------------------------------------------------
// |
// |  Parser
// |
//  ---------------------------------------------------------------

void readCsvHeader(FILE* file, char** headerInfo) {
	// pLx,pLy,pRx,pRy,PupilDistance,cLx,cLy,cRx,cRy,CornerDistance;
	int counter = 0;
	while (true) {
		char* header = new char[100];
		if (fscanf(file, "%[^\n,],", header))
			headerInfo[counter++] = header;
		else
			return;
	}
}

char** loopOverAllLogFiles(const char *path, const char *fExt, std::function<void(int distance, const char* extension, FILE* file, char** header)>func) {
	// read csv header strings
	char** header = new char*[NumOfCols];
	bool initHeader = true;
	
	for (int i = 0; i < 225; i++) {
		int distance = (i%200)+1;
		const char* ext = (i<200 ? "cm" : "m"); // 1-200cm & 1-25m
		char name[50];
		snprintf(name, 50*sizeof(char), "%d%s.%s.pupilpos.csv", distance, ext, fExt);
		
		FILE* file = openFile(path, name);
		// non existent files will be skipped automatically
		if (file) {
			if (initHeader) {
				readCsvHeader(file, header);
				header[10] = (char*)"ratio";
				initHeader = false;
			}
			func(distance, ext, file, header);
			fclose(file);
		}
	}
	
	if (initHeader == false)
		return header;
	return nullptr;
}

size_t scanFile(FILE* file, unsigned long skipFirstXValues, MinMaxAvg* mma, MinMaxAvg* additional = nullptr) {
	unsigned long skip = skipFirstXValues;
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
		val[10] = val[4] / val[9]; // ratio pupil to corner distance
		mma->update(val);
		if (additional)
			additional->update(val);
	}
	return mma->calculateAverage();
}



//  ---------------------------------------------------------------
// |
// |  Main
// |
//  ---------------------------------------------------------------

int main( int argc, const char** argv ) {
	unsigned long skipValues = 0;
	const char* videoExt = "MP4";
	const char* folder = NULL;
	
	// program argument evaluation
	for (int i = 1; i < argc; i++) {
		const char* param = argv[i];
		if (strcmp(param, "-s") == 0 || strcmp(param, "-skip") == 0) {
			if (i+1 < argc) {
				i++;
				skipValues = strtoul(argv[i], NULL, 10);
			}
		} else if (strcmp(param, "-e") == 0 || strcmp(param, "-ext") == 0) {
			if (i+1 < argc) {
				i++;
				videoExt = argv[i];
			}
		} else {
			folder = argv[i];
		}
	}
	
	if (folder == NULL) {
		fputs("Usage: logEvaluator {-skip 50} {-ext MP4} ../series7/ \n\n", stderr);
		return EXIT_FAILURE;
	}
//	printf("Processing folder '%s':\n", folder);
	
	MinMaxAvg global;
	Difference globalDiff;
	FILE* outErrFile = openFile(folder, file_error, true);
	FILE* report = openFile(folder, file_report, true);
	if (!outErrFile || !report) {
		printf("Couldn't create output file '%s' at '%s'.\n", (!outErrFile ? file_error : file_report), folder);
		return EXIT_FAILURE;
	}
	
	fprintf(outErrFile, "file,min,max,avg,max-min,avg-min,max-avg,type\n");
	fprintf(report, "\n  file |  pupil dist   |     ratio     | angle | samples ");
	fprintf(report, "\n-------+---------------+---------------+-------+---------\n");
	
	char** head = loopOverAllLogFiles(folder, videoExt, [&global,&globalDiff,&outErrFile,&report,skipValues](int fDist, const char* fExt, FILE* file, char** header)
	{
		MinMaxAvg local;
		size_t count = scanFile(file, skipValues, &local, &global);
		
		if (count > 0) {
			Difference diff = Difference(local);
			globalDiff.update(diff);
			
			float maxErrDist = std::fmax(diff.toMin[4], diff.toMax[4]);
			float maxErrRatio = std::fmax(diff.toMin[10], diff.toMax[10]);
			float pupilDistanceInMM = local.avg[10] * 35; // Hard coded 3.5cm eye corner distance
			float angle = 2 * atanf( (pupilDistanceInMM / 2.0f) / (fDist * 10) ) * 180 / M_PI;
			
			fprintf(report, " %3d%-2s | %6.2f ±%5.2f | %1.3f ± %1.3f | %5.2f | %4lu\n",
					fDist, fExt, local.avg[4], maxErrDist, local.avg[10], maxErrRatio, angle, count);
			
			for (int i = 0; i < NumOfCols; i++) {
				fprintf(outErrFile, "%d%s,%1.3f,%1.3f,%1.3f,%1.3f,%1.3f,%1.3f,%s\n",
						fDist, fExt, local.min[i], local.max[i], local.avg[i], diff.total[i], diff.toMin[i], diff.toMax[i], header[i]);
			}
		}
	});
	fclose(outErrFile);
	
	size_t totalNumberOfLines = global.calculateAverage();
	if (totalNumberOfLines > 0) {
		fprintf(report, "\nSamples total: %lu\n", totalNumberOfLines);
		fprintf(report, "  Pupil:  ( %6.2f -* %6.2f *- %6.2f )\n", global.min[4], global.avg[4], global.max[4]);
		fprintf(report, "  Corner: ( %6.2f -* %6.2f *- %6.2f )\n", global.min[9], global.avg[9], global.max[9]);
		fprintf(report, "  Ratio:  ( %6.3f -* %6.3f *- %6.3f )\n\n", global.min[10], global.avg[10], global.max[10]);
	}
	
	if (head) {
		fprintf(report, "Max variance:\n");
		for (int i = 0; i < NumOfCols; i++) {
			float maxErr = std::fmax(globalDiff.toMin[i], globalDiff.toMax[i]);
			fprintf(report, "%8.3f  %s\n", maxErr, head[i]);
		}
		fprintf(report, "\n");
	}
	fclose(report);
	
	printf("Writing file: %s\n", file_error);
	printf("Writing file: %s\n", file_report);
	printf("Done.\n\n");
	
	return EXIT_SUCCESS;
}

