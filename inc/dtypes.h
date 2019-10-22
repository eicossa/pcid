#include <string>

#ifndef AJRLICENSE_H_
#define AJRLICENSE_H_

#ifdef __cplusplus
extern "C" {
#endif

//definition of size_t
#include <stdlib.h>
#ifndef _MSC_VER
#include <stdbool.h>
#endif

#ifdef __unix__
#define DllExport
#else
#include <windows.h>
#define DllExport  __declspec( dllexport )
#endif

#define ENVIRONMENT_VAR_NAME_MAX 64
#define PC_IDENTIFIER_SIZE 18
#define PROPRIETARY_DATA_SIZE 16

#define LICENESE_INT_VERSION 100
#define LICENSEPP_VERSION "1.0.0"

typedef enum {
	LICENSE_OK                = 0, 						//OK
	LICENSE_FILE_NOT_FOUND    = 1, 		//license file not found
	LICENSE_SERVER_NOT_FOUND  = 2, 		//license server can't be contacted
	ENVIRONMENT_VARIABLE_NOT_DEFINED = 3, //environment variable not defined
	FILE_FORMAT_NOT_RECOGNIZED = 4,	//license file has invalid format (not .ini file)
	LICENSE_MALFORMED         = 5, //some mandatory field are missing, or data can't be fully read.
	PRODUCT_NOT_LICENSED      = 6,		//this product was not licensed
	PRODUCT_EXPIRED           = 7,
	LICENSE_CORRUPTED         = 8,//License signature didn't match with current license
	IDENTIFIERS_MISMATCH      = 9, //Calculated identifier and the one provided in license didn't match

	LICENSE_FILE_FOUND        = 100,
	LICENSE_VERIFIED          = 101

} EVENT_TYPE;

typedef enum {
	LOCAL, REMOTE //remote licenses are not supported now.
} LICENSE_TYPE;

typedef enum {
	SVRT_INFO, SVRT_WARN, SVRT_ERROR
} SEVERITY;

typedef struct {
	SEVERITY severity;
	EVENT_TYPE event_type;
	char param1[256];
	char param2[256];
} AuditEvent;


typedef struct {
	const char *licenseFileLocation;
	const char *environmentVariableName;
	bool openFileNearModule;
} LicenseLocation;

typedef struct {
	/**
	 * Detailed reason of success/failure. Reasons for a failure can be
	 * multiple (for instance, license expired and signature not verified).
	 * Only the last 5 are reported.
	 */
	AuditEvent status[5];
	/**
	 * Eventual expiration date of the software,
	 * can be '\0' if the software don't expire
	 * */
	char expiry_date[11];
	unsigned int days_left;bool has_expiry;bool linked_to_pc;
	LICENSE_TYPE license_type; // Local or Remote
	/* A string of character inserted into the license understood
	 * by the calling application.
	 * '\0' if the application didn't specify one */
	char proprietary_data[PROPRIETARY_DATA_SIZE + 1];
	int license_version; //license file version
} LicenseStatus;

/**
 * Enum to select a specific pc identification_strategy. DEFAULT Should be used
 * in most cases.
 */
typedef enum {
	DEFAULT,
	ETHERNET,
	IP_ADDRESS,
	DISK_NUM,
	DISK_LABEL,
	PLATFORM_SPECIFIC,
	STRATEGY_UNKNOWN
} IDENTIFICATION_STRATEGY;

#ifdef __cplusplus
}
#endif
#endif




#ifndef BASE_H_
#define BASE_H_


#ifdef __cplusplus
extern "C" {
#endif

#ifdef __unix__
#include <limits.h>
#define DllExport
#ifndef MAX_PATH
	#define MAX_PATH PATH_MAX
#endif

#else //windows
#include <windows.h>
#define DllExport  __declspec( dllexport )

#ifndef __cplusplus
#ifndef _MSC_VER
#include <stdbool.h>
#else
typedef int bool;
#define false 0
#define true -1
#endif
#endif

#endif
/* #define _DEBUG */

#define cmax(a,b) \
   ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a > _b ? _a : _b; })

#define cmin(a,b) \
   ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a < _b ? _a : _b; })

typedef enum  {
	FUNC_RET_OK, FUNC_RET_NOT_AVAIL, FUNC_RET_ERROR, FUNC_RET_BUFFER_TOO_SMALL
} FUNCTION_RETURN;

#ifdef __cplusplus
}
#endif

#endif


#ifndef PC_IDENTIFIERS_H_
#define PC_IDENTIFIERS_H_
//#include "api/datatypes.h"
//#include "base/base.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef unsigned char PcIdentifier[6];
typedef char PcSignature[21];

FUNCTION_RETURN generate_pc_id(PcIdentifier * identifiers, unsigned int * array_size, IDENTIFICATION_STRATEGY strategy);

EVENT_TYPE validate_pc_signature(PcSignature str_code);

/**
 * Generates an UserPcIdentifier.
 *
 * @param identifier_out
 * @param strategy
 * @return
 */
FUNCTION_RETURN generate_user_pc_signature(PcSignature identifier_out, IDENTIFICATION_STRATEGY strategy);

#ifdef __cplusplus
}
#endif
#endif /* PC_IDENTIFIERS_H_ */





#ifndef logger_INCLUDED
#define logger_INCLUDED

#ifndef LOG_DISABLED
#include <errno.h>
#ifdef __cplusplus
extern "C" {
#endif

#define clean_errno() (errno == 0 ? "None" : strerror(errno))

#ifdef _DEBUG
#define LOG_DEBUG(M, ...) _log("[DEBUG] (%s:%d) " M "\n", __FILE__, __LINE__, ##__VA_ARGS__)
#else
#define LOG_DEBUG(M,...)
#endif
#define LOG_INFO(M, ...) _log("[INFO] (%s:%d) " M "\n", __FILE__, __LINE__, ##__VA_ARGS__)
#define LOG_WARN(M, ...) _log("[WARN] (%s:%d: errno: %s) " M "\n", __FILE__, __LINE__, clean_errno(), ##__VA_ARGS__)
#define LOG_ERROR(M, ...) _log("[ERROR] (%s:%d: errno: %s) " M "\n", __FILE__, __LINE__, clean_errno(), ##__VA_ARGS__)

#else
#define LOG_DEBUG(M,...)
#define LOG_INFO(M, ...)
#define LOG_WARN(M, ...)
#define LOG_ERROR(M, ...)
#endif

void _log(const char* format, ...);
void _shutdown_log();

#ifdef __cplusplus
}
#endif
#endif



/*

 https://github.com/superwills/NibbleAndAHalf
 base64.h -- Fast base64 encoding and decoding.
 version 1.0.0, April 17, 2013 143a

 Copyright (C) 2013 William Sherif

 This software is provided 'as-is', without any express or implied
 warranty.  In no event will the authors be held liable for any damages
 arising from the use of this software.

 Permission is granted to anyone to use this software for any purpose,
 including commercial applications, and to alter it and redistribute it
 freely, subject to the following restrictions:

 1. The origin of this software must not be misrepresented; you must not
 claim that you wrote the original software. If you use this software
 in a product, an acknowledgment in the product documentation would be
 appreciated but is not required.
 2. Altered source versions must be plainly marked as such, and must not be
 misrepresented as being the original software.
 3. This notice may not be removed or altered from any source distribution.

 William Sherif
 will.sherif@gmail.com

 YWxsIHlvdXIgYmFzZSBhcmUgYmVsb25nIHRvIHVz

 */
#ifndef BASE64_H
#define BASE64_H

#include <stdio.h>
#include <stdlib.h>

const static char* b64 =
		"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

// maps A=>0,B=>1..
const static unsigned char unb64[] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //10
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //20
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //30
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //40
		0, 0, 0, 62, 0, 0, 0, 63, 52, 53, //50
		54, 55, 56, 57, 58, 59, 60, 61, 0, 0, //60
		0, 0, 0, 0, 0, 0, 1, 2, 3, 4, //70
		5, 6, 7, 8, 9, 10, 11, 12, 13, 14, //80
		15, 16, 17, 18, 19, 20, 21, 22, 23, 24, //90
		25, 0, 0, 0, 0, 0, 0, 26, 27, 28, //100
		29, 30, 31, 32, 33, 34, 35, 36, 37, 38, //110
		39, 40, 41, 42, 43, 44, 45, 46, 47, 48, //120
		49, 50, 51, 0, 0, 0, 0, 0, 0, 0, //130
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //140
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //150
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //160
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //170
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //180
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //190
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //200
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //210
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //220
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //230
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //240
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //250
		0, 0, 0, 0, 0, 0, }; // This array has 255 elements

char* base64(const void* binaryData, int len, int *flen);
unsigned char* unbase64(const char* ascii, int len, int *flen);
#endif





#ifndef PC_IDENTIFIERS_H_
#define PC_IDENTIFIERS_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef unsigned char PcIdentifier[6];
typedef char PcSignature[21];

FUNCTION_RETURN generate_pc_id(PcIdentifier * identifiers, unsigned int * array_size,
		IDENTIFICATION_STRATEGY strategy);

EVENT_TYPE validate_pc_signature(PcSignature str_code);

/**
 * Generates an UserPcIdentifier.
 *
 * @param identifier_out
 * @param strategy
 * @return
 */
FUNCTION_RETURN generate_user_pc_signature(PcSignature identifier_out,
		IDENTIFICATION_STRATEGY strategy);

#ifdef __cplusplus
}
#endif
#endif /* PC_IDENTIFIERS_H_ */



#ifndef OS_DEPENDENT_HPP_
#define OS_DEPENDENT_HPP_

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
//definition of size_t
#include <stdlib.h>
#ifdef __unix__
#include <unistd.h>
#include <stdbool.h>
#endif

typedef enum {
	NONE, VMWARE
} VIRTUALIZATION;

typedef enum {
	IFACE_TYPE_ETHERNET, IFACE_TYPE_WIRELESS
} IFACE_TYPE;

typedef struct {
	int id;
	char description[1024];
	unsigned char mac_address[8];
	unsigned char ipv4_address[4];
	IFACE_TYPE type;
} OsAdapterInfo;

typedef struct {
	int id;
	char device[255];
	unsigned char disk_sn[8];
	char label[255];
	bool preferred;
} DiskInfo;

FUNCTION_RETURN getAdapterInfos(OsAdapterInfo * adapterInfos, size_t * adapter_info_size);
FUNCTION_RETURN getDiskInfos(DiskInfo * diskInfos, size_t * disk_info_size);
FUNCTION_RETURN getUserHomePath(char[MAX_PATH]);
FUNCTION_RETURN getModuleName(char buffer[MAX_PATH]);
FUNCTION_RETURN getCpuId(unsigned char identifier[6]);
FUNCTION_RETURN getMachineName(unsigned char identifier[6]);
/**
 * Get an identifier of the machine in an os specific way.
 * In Linux it uses:
 * http://stackoverflow.com/questions/328936/getting-a-unique-id-from-a-unix-like-system
 *
 * <ul>
 * <li>Dbus if available</li>
 * </ul>
 * Can be used as a fallback in case no other methods are available.
 * Windows:
 * HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows NT\ CurrentVersion\ProductId
 * http://sowkot.blogspot.it/2008/08/generating-unique-keyfinger-print-for.html
 * http://stackoverflow.com/questions/2842116/reliable-way-of-generating-unique-hardware-id
 *
 *
 * @param identifier
 * @return
 */
FUNCTION_RETURN getOsSpecificIdentifier(unsigned char identifier[6]);
VIRTUALIZATION getVirtualization();
void os_initialize();

FUNCTION_RETURN verifySignature(const char* stringToVerify, const char* signatureB64);

#ifdef __cplusplus
}
#endif

#endif /* OS_DEPENDENT_HPP_ */
