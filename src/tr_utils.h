/*
 * Copyright 2018-2019 by Juby Abraham
 * GPS Tracking Student. 
 * Emulate Infotech Pvt. Ltd.
 *
 */

#ifndef TR_UTILS
#define TR_UTILS

#include <time.h>

/*#define DEBUG*/

#define pi 					3.1415926535897932384626433832795
#define degPerRad			57.295779513082320876798154814105
#define	radPerDeg		0.01745329251994329576923690768489

typedef enum {FALSE=0, TRUE=1} boolean;

typedef unsigned long long int uInt64;
typedef long long int Int64;
typedef unsigned int uInt32;
typedef unsigned short int uInt16;
typedef unsigned char uInt8;
typedef int Int32;
typedef short int Int16;
typedef char Int8;


//service functions:
uInt32 printHEXandCHARS(FILE* dev, const Int8 *adr, uInt32 offset, uInt32 len, boolean printChars);
void printHexBytes(FILE* dev, Int8 *adr, uInt32 n);
uInt16 swapU16(uInt16 x);
uInt32 swapU32(uInt32 x);
Int16 swap16(Int16 x);
Int32 swap32(Int32 x);
char* time2str(char* str);
void strreplace(char s[], char chr, char repl_chr);
char* uInt8toStr(char* dst, unsigned char* src, int len);

#endif /* TR_UTILS */

