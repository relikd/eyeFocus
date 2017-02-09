#include <stdio.h>
#include <stdlib.h>
#include <cstring>
#include <functional>
#include <string>


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

void loopOverAllLogFiles(const char *path, std::function<std::string(int distance, const char* extension, FILE* file)>func) {
	for (int i = 0; i < 225; i++) {
		int distance = (i%200)+1;
		const char* ext = (i<200 ? "cm" : "m"); // 1-200cm & 1-25m
		char name[50];
		snprintf(name, 50*sizeof(char), "%d%s.MP4.eyepos.txt", distance, ext);
		
		FILE* file = openFile(path, name);
		// non existent files will be skipped automatically
		if (file) {
			std::string str = func(distance, ext, file);
			fclose(file);
			file = openFile(path, name, true);
			if (file) {
				fprintf(file, "%s", str.c_str());
				fclose(file);
			}
		}
	}
}

//  ---------------------------------------------------------------
// |
// |  Main
// |
//  ---------------------------------------------------------------

int main( int argc, const char** argv ) {
	if (argc != 2) {
		fputs("Missing argument value. Path to folder containing eyeFocus log files expected.\n\n", stderr);
		return EXIT_SUCCESS;
	}
	const char* folder = argv[1];
	printf("Processing folder '%s':\n", folder);
	
	loopOverAllLogFiles(folder, [](int fDist, const char* fExt, FILE* file)
	{
//		Eye Area:
//		[167 192] [241 196] - [788 168] [713 174]
//		Eye Corner:
//		[314 206] [653 188]
		char str[200] = "";
		while (true) {
			char tmp[50];
			int val;
			int a = fscanf(file, "%d[^ ]", &val);
			if (a > 0) {
				static bool t = true;
				snprintf(tmp, 50*sizeof(char), (t ? "%d " : "%d"), val*2);
				t = !t;
			} else {
				int b = fscanf(file, "%[^0-9]", tmp);
				if (a == EOF || b == EOF) {
					break;
				}
			}
			strcat(str, tmp);
		}
		return std::string(str);
	});
	
	
	return EXIT_SUCCESS;
}

