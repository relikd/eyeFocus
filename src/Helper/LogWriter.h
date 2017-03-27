//
//  LogWriter.h
//  eyeFocus
//
//  Created by Oleg Geier on 07/02/17.
//
//

#ifndef LogWriter_h
#define LogWriter_h

#include "FileIO.h"

class LogWriter {
	FILE* file = NULL;
	
public:
	/** Write an optional .csv file. @param header First line of file */
	LogWriter(const char* path = NULL, const char* header = NULL) {
		if (path) {
			file = FileIO::openFile(path, true);
			if (file && header) fprintf(file, "%s", header);
		}
	};
	
	/** Creates console output and if path provided an additional .csv file */
	void writePointPair(cv::Point2f a, cv::Point2f b, bool isCorner) {
		double dist = cv::norm(a - b);
		if (isCorner) printf(" | ");
		printf("%s: ([%1.1f,%1.1f],[%1.1f,%1.1f], dist: %1.1f)", (isCorner?"corner":"pupil"), a.x, a.y, b.x, b.y, dist);
		if (isCorner) printf("\n");
		if (file) fprintf(file, "%1.4f,%1.4f,%1.4f,%1.4f,%1.2f%c", a.x, a.y, b.x, b.y, dist, (isCorner?'\n':','));
	}
	
	~LogWriter() {
		if (file) fclose(file);
	}
};

#endif /* LogWriter_h */
