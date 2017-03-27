//
//  FileIO.h
//  eyeFocus
//
//  Created by Oleg Geier on 27/03/17.
//
//

#ifndef FileIO_h
#define FileIO_h

#ifdef _WIN32
	#include <windows.h> /* defines CreateDirectory() */
	#include <direct.h>
	#define cwd _getcwd
	#define cd _chdir
#else
	#include <sys/stat.h> /* defines mkdir() */
	#include "unistd.h"
	#define cwd getcwd
	#define cd chdir
#endif

#include <stdio.h>  /* defines FILENAME_MAX */
#include <string>

class FileIO {
public:
	
	//  ---------------------------------------------------------------
	// |  Directories
	//  ---------------------------------------------------------------
	
	static std::string currentWorkingDirectory() {
		char buf[FILENAME_MAX];
		char* fp = cwd(buf, sizeof(buf));
		return std::string(fp);
	}

	static int changeDirectory ( const char * path ) {
		return cd(path);
	}
	
	static void createDirectory ( const char * path ) {
#ifdef _WIN32
		CreateDirectory(path, NULL);
#else
		mkdir(path, 0700);
#endif
	}
	
	//  ---------------------------------------------------------------
	// |  Files
	//  ---------------------------------------------------------------
	
	static FILE* openFile ( const char * path, bool write = false ) {
		if (path) {
			FILE* file;
#ifdef _WIN32
			fopen_s(&file, path, (write?"w":"r"));
#else
			file = fopen(path, (write?"w":"r"));
#endif
			return file;
		}
		return NULL;
	}
	
	static std::string str ( const char * format, ... ) {
		char buffer[4095];
		va_list args;
		va_start (args, format);
		vsprintf (buffer, format, args);
		std::string s(buffer);
		va_end (args);
		return s;
	}
};

#endif /* FileIO_h */
