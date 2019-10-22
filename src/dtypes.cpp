#include "dtypes.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>

#ifdef __unix__
#include <unistd.h>
#define MAX_PATH 255
#else
#include <windows.h>
#endif

static FILE *logFile = NULL;

static void timenow(char * buffer) {
	time_t rawtime;
	struct tm *timeinfo;

	time(&rawtime);
	timeinfo = localtime(&rawtime);

	strftime(buffer, 64, "%Y-%m-%d %H:%M:%S", timeinfo);
}

static void getLogFname(char* logpath) {
#ifdef __unix__
	char const *folder = getenv("TMPDIR");
	if (folder == 0) {
		folder = "/tmp";
	}
	strncpy(logpath, folder, MAX_PATH);
	strncat(logpath, "/open-license.log", MAX_PATH - strlen(logpath));
#else
	int plen = GetTempPath(MAX_PATH, logpath);
	if(plen == 0) {
		fprintf(stderr, "Error getting temporary directory path");
	}
	strncat(logpath, "open-license.log", MAX_PATH - strlen(logpath));
#endif
}

void _log(const char* format, ...) {
	char logpath[MAX_PATH];
	va_list args;
	char * buffer;
	if (logFile == NULL) {
		getLogFname(logpath);
		logFile = fopen(logpath, "a");
		if (logFile == NULL) {
			//what shall we do here?
			return;
		}
	}
	buffer = (char *) malloc(sizeof(char) * strlen(format) + 64);
	timenow(buffer);
	sprintf(&buffer[strlen(buffer) - 1], "-[%d]-", getpid());
	strcat(buffer, format);
	va_start(args, format);
	vfprintf(logFile, buffer, args);
	va_end(args);
	free(buffer);
}

void _shutdown_log() {
	if (logFile != NULL) {
		fclose(logFile);
		logFile = NULL;
	}
}



// Converts binary data of length=len to base64 characters.
// Length of the resultant string is stored in flen
// (you must pass pointer flen).
char* base64(const void* binaryData, int len, int *flen) {
	const unsigned char* bin = (const unsigned char*) binaryData;
	char* res;

	int rc = 0; // result counter
	int byteNo; // I need this after the loop

	int modulusLen = len % 3;
	int pad = ((modulusLen & 1) << 1) + ((modulusLen & 2) >> 1); // 2 gives 1 and 1 gives 2, but 0 gives 0.

	*flen = 4 * (len + pad) / 3;
	res = (char*) malloc(*flen + 1); // and one for the null
	if (!res) {
		puts("ERROR: base64 could not allocate enough memory.");
		puts("I must stop because I could not get enough");
		return 0;
	}

	for (byteNo = 0; byteNo <= len - 3; byteNo += 3) {
		unsigned char BYTE0 = bin[byteNo];
		unsigned char BYTE1 = bin[byteNo + 1];
		unsigned char BYTE2 = bin[byteNo + 2];
		res[rc++] = b64[BYTE0 >> 2];
		res[rc++] = b64[((0x3 & BYTE0) << 4) + (BYTE1 >> 4)];
		res[rc++] = b64[((0x0f & BYTE1) << 2) + (BYTE2 >> 6)];
		res[rc++] = b64[0x3f & BYTE2];
	}

	if (pad == 2) {
		res[rc++] = b64[bin[byteNo] >> 2];
		res[rc++] = b64[(0x3 & bin[byteNo]) << 4];
		res[rc++] = '=';
		res[rc++] = '=';
	} else if (pad == 1) {
		res[rc++] = b64[bin[byteNo] >> 2];
		res[rc++] = b64[((0x3 & bin[byteNo]) << 4) + (bin[byteNo + 1] >> 4)];
		res[rc++] = b64[(0x0f & bin[byteNo + 1]) << 2];
		res[rc++] = '=';
	}

	res[rc] = 0; // NULL TERMINATOR! ;)
	return res;
}

unsigned char* unbase64(const char* ascii, int len, int *flen) {
	const unsigned char *safeAsciiPtr = (const unsigned char*) ascii;
	unsigned char *bin;
	int cb = 0;
	int charNo;
	int pad = 0;

	if (len < 2) { // 2 accesses below would be OOB.
		// catch empty string, return NULL as result.
		puts(
				"ERROR: You passed an invalid base64 string (too short). You get NULL back.");
		*flen = 0;
		return 0;
	}
	if (safeAsciiPtr[len - 1] == '=')
		++pad;
	if (safeAsciiPtr[len - 2] == '=')
		++pad;

	*flen = 3 * len / 4 - pad;
	bin = (unsigned char*) malloc(*flen);
	if (!bin) {
		puts("ERROR: unbase64 could not allocate enough memory.");
		puts("I must stop because I could not get enough");
		return 0;
	}

	for (charNo = 0; charNo <= len - 4 - pad; charNo += 4) {
		int A = unb64[safeAsciiPtr[charNo]];
		int B = unb64[safeAsciiPtr[charNo + 1]];
		int C = unb64[safeAsciiPtr[charNo + 2]];
		int D = unb64[safeAsciiPtr[charNo + 3]];

		bin[cb++] = (A << 2) | (B >> 4);
		bin[cb++] = (B << 4) | (C >> 2);
		bin[cb++] = (C << 6) | (D);
	}

	if (pad == 1) {
		int A = unb64[safeAsciiPtr[charNo]];
		int B = unb64[safeAsciiPtr[charNo + 1]];
		int C = unb64[safeAsciiPtr[charNo + 2]];

		bin[cb++] = (A << 2) | (B >> 4);
		bin[cb++] = (B << 4) | (C >> 2);
	} else if (pad == 2) {
		int A = unb64[safeAsciiPtr[charNo]];
		int B = unb64[safeAsciiPtr[charNo + 1]];

		bin[cb++] = (A << 2) | (B >> 4);
	}

	return bin;
}