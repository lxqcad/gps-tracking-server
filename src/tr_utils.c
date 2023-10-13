/*
 * Copyright 2018-2019 by Juby Abraham
 * GPS Tracking Student. 
 * Emulate Infotech Pvt. Ltd.
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "tr_protocol.h"
#include "tr_utils.h"

uInt32 printHEXandCHARS(FILE* dev, const Int8 *adr, uInt32 offset, uInt32 len, boolean printChars) {
   register uInt32 j, k;
   const uInt32 deflen = 16;
   char str[deflen];
   char c;

   len = (len >= deflen) ? deflen : len;

   for(j = offset, k = 0; j < (offset + len); ++j, ++k) {
      c = (char)adr[j];
      /*if(c < 16) 
      fprintf(dev, " "); */
      fprintf(dev, " %02X", (unsigned char)adr[j]);
      if(c >= 32 && c <= 126)
         str[k] = c;
      else
         str[k] = '.';
   }
   offset += len;
   if(printChars) {
      fprintf(dev, "   ");
      for(j = 0; j < len; ++j)
         fprintf(dev, "%c", str[j]);
   }
   return offset;
}

void printHexBytes(FILE* dev, Int8 *adr, uInt32 n) {
   register uInt32 i;
  
   //Serial.println("");
   for(i = 0; i < n;) {
      fprintf(dev, "%p ", &adr[i]);
      //fprintf(dev, " ");
      i = printHEXandCHARS(dev, adr, i, 16, TRUE);
      fprintf(dev, "\n");
   }
}

inline uInt16 swapU16(uInt16 x) {
   return ((x>>8) | (x<<8));
}

inline Int16 swap16(Int16 x) {
   return ((x>>8) | (x<<8));
}

inline uInt32 swapU32(uInt32 x) {
   uInt32 low = swapU16(x & 0xFFFF), high = swapU16((x>>16) & 0xFFFF);
   return ((low<<16) | high);
}

inline Int32 swap32(Int32 x) {
   Int32 low = swap16(x & 0xFFFF), high = swap16((x>>16) & 0xFFFF);
   return ((low<<16) | high);
}

inline char* time2str(char* str) {
   time_t current_time = time(NULL);
   strcpy(str, ctime(&current_time));
   // fix the bug with end of line
   strreplace(str, '\r', '\0');
   strreplace(str, '\n', '\0');
   return str;
}

void strreplace(char s[], char chr, char repl_chr) {
   int i = 0;
   while(s[i] != '\0') {
      if(s[i] == chr) {
         s[i] = repl_chr;
      }
      i++; 
   }
}

inline char* uInt8toStr(char* dst, unsigned char* src, int len) {
   int i;
   for(i = 0; i < len; ++i) {
      #ifndef SWITCH_SECURE_IMEI
         sprintf(dst+i*2, "%02X", src[i]);
      #else
         if(i < (len - 1) && i > 1)
            sprintf(dst+i*2, "**");
         else
            sprintf(dst+i*2, "%02X", src[i]);
      #endif
   }
   *(dst + len * 2) = '\0';
   return dst;
}


