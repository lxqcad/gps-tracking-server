/*
 * Copyright 2018-2019 by Juby Abraham
 * GPS Tracking Student. 
 * Emulate Infotech Pvt. Ltd.
 *
 */
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include "tr_protocol.h"
#include "tr_utils.h"

#define HB_BITS	5
static t_PHB pHB[HB_BITS] = {
   {1, "Def. off/on:"},
   {1<<1, "ACC low/high:"},
   {1<<2, "Charge off/on:"},
   {1<<6, "GPS track. off/on:"},
   {1<<7, "Relay off/on:"}
};

// An attempt to make a parameters (names) list and link to another naming structure 
static t_PNames pNames[SUPPORTED_P] = {
   {.pNo = PLogINFO, .pName = "<Login>", .ic = NULL, .sz = sizeof(t_PLoginMSG)},
   {.pNo = PPosDAT, .pName = "<Position>", .ic = NULL, .sz = sizeof(t_PLocation)},
   {.pNo = PStatInfo, .pName = "<Heartbeat>", .ic = &pHB, .sz = sizeof(t_PHeartBeatMSG)},
   {.pNo = PAlarmDAT, .pName = "<Alarm>", .ic = &pHB, .sz = sizeof(t_PAlarmMSG)},
   {.pNo = PGPSAdrQ, .pName = "<GPS addr. query pckt.>", .ic = NULL, .sz = sizeof(0)},
   {.pNo = PLBSAdrQ, .pName = "<LBS addr. query pckt.>", .ic = NULL, .sz = sizeof(0)},
   {.pNo = PLBSInfo, .pName = "<LBS multi base statio ext. info>", .ic = NULL, .sz = sizeof(0)},
   {.pNo = PDemInfo, .pName = "<Online cmd. sent to term.>", .ic = NULL, .sz = sizeof(t_POnlineCMD)},
   {.pNo = PCMDResp, .pName = "<Online cmd. response by term.>", .ic = NULL, .sz = sizeof(0)},
   {.pNo = PTimeCalib, .pName = "<Time calib. pckt.>", .ic = NULL, .sz = sizeof(0)},
   {.pNo = PInfoTrans, .pName = "<Info. trans. pckt.>", .ic = NULL, .sz = sizeof(0)}
};
 
//code fragment of the CRC-ITU lookup table algorithm implemented based on C language 
static const uInt16 crctab16[] = { 
   0x0000, 0x1189, 0x2312, 0x329B, 0x4624, 0x57AD, 0x6536, 0x74BF, 
   0x8C48, 0x9DC1, 0xAF5A, 0xBED3, 0xCA6C, 0xDBE5, 0xE97E, 0xF8F7, 
   0x1081, 0x0108, 0x3393, 0x221A, 0x56A5, 0x472C, 0x75B7, 0x643E, 
   0x9CC9, 0x8D40, 0xBFDB, 0xAE52, 0xDAED, 0xCB64, 0xF9FF, 0xE876, 
   0x2102, 0x308B, 0x0210, 0x1399, 0x6726, 0x76AF, 0x4434, 0x55BD, 
   0xAD4A, 0xBCC3, 0x8E58, 0x9FD1, 0xEB6E, 0xFAE7, 0xC87C, 0xD9F5, 
   0x3183, 0x200A, 0x1291, 0x0318, 0x77A7, 0x662E, 0x54B5, 0x453C, 
   0xBDCB, 0xAC42, 0x9ED9, 0x8F50, 0xFBEF, 0xEA66, 0xD8FD, 0xC974, 
   0x4204, 0x538D, 0x6116, 0x709F, 0x0420, 0x15A9, 0x2732, 0x36BB, 
   0xCE4C, 0xDFC5, 0xED5E, 0xFCD7, 0x8868, 0x99E1, 0xAB7A, 0xBAF3, 
   0x5285, 0x430C, 0x7197, 0x601E, 0x14A1, 0x0528, 0x37B3, 0x263A, 
   0xDECD, 0xCF44, 0xFDDF, 0xEC56, 0x98E9, 0x8960, 0xBBFB, 0xAA72, 
   0x6306, 0x728F, 0x4014, 0x519D, 0x2522, 0x34AB, 0x0630, 0x17B9, 
   0xEF4E, 0xFEC7, 0xCC5C, 0xDDD5, 0xA96A, 0xB8E3, 0x8A78, 0x9BF1, 
   0x7387, 0x620E, 0x5095, 0x411C, 0x35A3, 0x242A, 0x16B1, 0x0738, 
   0xFFCF, 0xEE46, 0xDCDD, 0xCD54, 0xB9EB, 0xA862, 0x9AF9, 0x8B70, 
   0x8408, 0x9581, 0xA71A, 0xB693, 0xC22C, 0xD3A5, 0xE13E, 0xF0B7, 
   0x0840, 0x19C9, 0x2B52, 0x3ADB, 0x4E64, 0x5FED, 0x6D76, 0x7CFF, 
   0x9489, 0x8500, 0xB79B, 0xA612, 0xD2AD, 0xC324, 0xF1BF, 0xE036, 
   0x18C1, 0x0948, 0x3BD3, 0x2A5A, 0x5EE5, 0x4F6C, 0x7DF7, 0x6C7E, 
   0xA50A, 0xB483, 0x8618, 0x9791, 0xE32E, 0xF2A7, 0xC03C, 0xD1B5, 
   0x2942, 0x38CB, 0x0A50, 0x1BD9, 0x6F66, 0x7EEF, 0x4C74, 0x5DFD, 
   0xB58B, 0xA402, 0x9699, 0x8710, 0xF3AF, 0xE226, 0xD0BD, 0xC134, 
   0x39C3, 0x284A, 0x1AD1, 0x0B58, 0x7FE7, 0x6E6E, 0x5CF5, 0x4D7C, 
   0xC60C, 0xD785, 0xE51E, 0xF497, 0x8028, 0x91A1, 0xA33A, 0xB2B3, 
   0x4A44, 0x5BCD, 0x6956, 0x78DF, 0x0C60, 0x1DE9, 0x2F72, 0x3EFB, 
   0xD68D, 0xC704, 0xF59F, 0xE416, 0x90A9, 0x8120, 0xB3BB, 0xA232,
   0x5AC5, 0x4B4C, 0x79D7, 0x685E, 0x1CE1, 0x0D68, 0x3FF3, 0x2E7A, 
   0xE70E, 0xF687, 0xC41C, 0xD595, 0xA12A, 0xB0A3, 0x8238, 0x93B1, 
   0x6B46, 0x7ACF, 0x4854, 0x59DD, 0x2D62, 0x3CEB, 0x0E70, 0x1FF9, 
   0xF78F, 0xE606, 0xD49D, 0xC514, 0xB1AB, 0xA022, 0x92B9, 0x8330, 
   0x7BC7, 0x6A4E, 0x58D5, 0x495C, 0x3DE3, 0x2C6A, 0x1EF1, 0x0F78, 
}; 

// returns P_* errors
int packet_handler(char* data_in, int n_in, char* data_out, int* n_out, char* imei) {
   char time[100];
   int res = P_NOERROR;
   t_PCommonHeader *ch = (t_PCommonHeader*)data_in;
   t_PCommonHeader2 *ch2;
   
   time2str(time);
   #ifdef DEBUG
      fprintf(stderr, "\nDEBUG: time-stamp %s", time);
      fprintf(stderr, "\nDEBUG: Received packet of length %d bytes:\n", n_in);
      printHexBytes(stderr, data_in, (n_in<MAX_ALLOWED_LEN)?n_in:MAX_ALLOWED_LEN);
   #endif
   
   // determine if packet is valid:
   switch(ch->startBit) {
      case StartBIT:
         // determine so called "protocol" (really?)
         switch(ch->pNo) {
            // cases of variable in length packets
            case PCMDResp:
               fprintf(stderr, "\nERROR: Packet 0x%02X is not implemented\n", ch->pNo);
               break;
            default:
               // normal packet arrival
               res = packet_reply_former(data_in, n_in, data_out, n_out, imei);
         }
         break;
      case StartBIT2:
         // special case:
         ch2 = (t_PCommonHeader2*)data_in;
         switch(ch2->pNo) {
            case PInfoTrans:
               // no response needed
               break;
            default:
             fprintf(stderr, "\nERROR: Packet 0x%02X is not implemented\n", ch2->pNo);
         }
         break;
      default:
         fprintf(stderr, "\nERROR: Unknown packet type 0x%04X\n", ch->startBit);
         res = P_UNKNOWN_TYPE;
   }
   if(res == P_NOERROR)
      packetLogger(stdout, data_in, n_in, imei);
   else
      fprintf(stdout, "\nERROR in p.h. %s, code=%3d", time, res);
   
   return res;
}

/* the main point with following function is to print out 
  content of the incoming packet from the terminal, i.e. 
  log the incoming packet and NOT outcoming!
 */
void packetLogger(FILE* dev, char* data_in, int n_in, char* imei) {
   char time[50], tstr[300], strCommand[220];
   t_StartBit sb = *((t_StartBit*)data_in);
   t_PCommonHeader *ch;
   t_PCommonHeader2 *ch2;
   int idx;
#ifndef SWITCH_SECURE_PInfoTrans
   uInt32 n;
   Int8 *cptr;
#endif
   
   #ifdef DEBUG
      fprintf(stderr, "\n|-----DEBUG START in packetLogger-----");
      fprintf(stderr, "\n| char* data_in=\t%p", data_in);
      fprintf(stderr, "\n|-----DEBUG END in packetLogger  -----\n");
   #endif
   
   time2str(time);
   
   fprintf(dev, "\n%s\tIMEI\t%s\t", time, (strlen(imei)?imei:"NONE"));
   switch(sb) {
      case StartBIT:
         ch = (t_PCommonHeader*)data_in;
         fprintf(dev, "P#\t0x%02X\t", ch->pNo);
         switch(ch->pNo) {
            case PLogINFO:
               idx = 0;
               fprintf(dev, "%s\t", pNames[idx].pName);
               fprintf(dev, "%s", uInt8toStr(tstr, ((t_PLoginMSG*)data_in)->imei, IMEI_LEN));
               break;
            case PPosDAT:
               // ignore for now the "mileage" version of this packet
               idx = 1;
               sprintf(strCommand,"php /home/ec2-user/gts/gps-data-interface.php gps %s %.8f %.8f %d", imei, (float)swapU32(((t_PLocation*)data_in)->x.lat)/1.8e6, (float)swapU32(((t_PLocation*)data_in)->x.lon)/1.8e6, (int)((t_PLocation*)data_in)->x.speed);
               system(strCommand);
               t_TimeLocation_printer(dev, idx, &(((t_PLocation*)data_in)->x));
               t_Pmob_Info_printer(dev, idx, &(((t_PLocation*)data_in)->mob));
               break;
            case PStatInfo:
               idx = 2;
               sprintf(strCommand,"php /home/ec2-user/gts/gps-data-interface.php heartbeat %s %d %d", imei, (int)((t_PHeartBeatMSG*)data_in)->infoCont.voltageLevel, (int)((t_PHeartBeatMSG*)data_in)->infoCont.gsmLevel);
               //fprintf(dev, "\n%s", strCommand);
               system(strCommand);
               t_SignalsAlarm_printer(dev, idx, &(((t_PHeartBeatMSG*)data_in)->infoCont));
               break;
            case PAlarmDAT:
               idx = 3;
               sprintf(strCommand,"php /home/ec2-user/gts/gps-data-interface.php alarm %s %.8f %.8f %d", imei, (float)swapU32(((t_PAlarmMSG*)data_in)->x.lat)/1.8e6, (float)swapU32(((t_PAlarmMSG*)data_in)->x.lon)/1.8e6, (int)((t_PAlarmMSG*)data_in)->alarmCont.alarm);
               system(strCommand);
               t_TimeLocation_printer(dev, idx, &(((t_PAlarmMSG*)data_in)->x));
               fprintf(dev, "\tLSB_LEN\t%d\t",(int)((t_PAlarmMSG*)data_in)->lbs_len);
               t_Pmob_Info_printer(dev, idx, &(((t_PAlarmMSG*)data_in)->mob));
               t_SignalsAlarm_printer(dev, idx, &(((t_PAlarmMSG*)data_in)->alarmCont));
               break;
            case PTimeCalib:
               idx = 9;
               fprintf(dev, "%s", pNames[idx].pName);
               break;
            default:
               fprintf(dev, "(unknown)");
         }
         //fprintf(dev, "\n");
         break;
      case StartBIT2:
         ch2 = (t_PCommonHeader2*)data_in;
         fprintf(dev, "PNo\t0x%02X\t", ch2->pNo);
         switch(ch2->pNo) {
            case PInfoTrans:
               idx = 10;
               fprintf(dev, "%s\t", pNames[idx].pName);
               fprintf(dev, "Info/SubP\t0x%02X\t", (int)((t_PInfoTransMSG*)data_in)->infoCont.subP);
               #ifndef SWITCH_SECURE_PInfoTrans
                  cptr = ((t_PInfoTransMSG*)data_in)->infoCont.unknown_or_variable;
                  n = n_in -
                     (sizeof(uInt8) + sizeof(t_PCommonHeader2) + sizeof(t_InfoSerNo) + 
                     sizeof(t_ErrChk) + sizeof(t_StopBit));
                  memset(tstr, 0, 300);
                  strncpy(tstr, cptr, (n>299)?299:n);
                  fprintf(dev, "DATA\t%s", tstr);
               #else
                  fprintf(dev, "DATA\t%s", "<xxx_secured_xxx>");
               #endif
               break;
            default:
               fprintf(dev, "(unknown)");
         }
         //fprintf(dev, "\n");
         break;
      default:
         fprintf(dev, "Unknown \"start bit\" packet %04X!", sb);
   }
   fflush(dev);
}

/* the following is valid for StartBit = 0x7878
because 0x7979 no need response from server
 Important: byte SWAP is important here! */
int packet_reply_former(char* data_in, int n_in, char* data_out, int* n_out, char* imei) {
   int res = P_NOERROR;
   uInt16 crcCalc;
   t_PCommonHeader *ch_tmp = (t_PCommonHeader*)data_in;
   t_PCommonRESP *cr = (t_PCommonRESP*)data_out;

   #ifdef DEBUG2
      fprintf(stderr, "\n|-----DEBUG START in packet_reply_former-----");
      fprintf(stderr, "\n| adr(ch_tmp->pNo)=\t%p", &ch_tmp->pNo);
      fprintf(stderr, "\n|ch_tmp->pNo=\t\t0x%02X", ch_tmp->pNo);
      fprintf(stderr, "\n|-----DEBUG END in packet_reply_former  -----\n");
   #endif
   
   if(checkCRC(data_in, n_in, &crcCalc)) {
      fprintf(stderr, "\nINFO: CRC of incoming packet is OK!\n");
      
      // these are common parameters for 3 packets:
      cr->ch.startBit = StartBIT;
      cr->ch.len = sizeof(t_PCommonRESP) - (sizeof(t_StartBit) + sizeof(t_PLen) + sizeof(t_StopBit)); 
      cr->ch.pNo = ch_tmp->pNo;
      cr->stopBit = swapU16(StopBIT);
      *n_out = sizeof(t_PCommonRESP);
      
      // unique settings:
      switch(ch_tmp->pNo) {
         case PLogINFO:
            cr->serNo = ((t_PLoginMSG*)data_in)->serNo;
            uInt8toStr(imei, ((t_PLoginMSG*)data_in)->imei, IMEI_LEN);
            break;
         case PStatInfo:
            cr->serNo = ((t_PHeartBeatMSG*)data_in)->serNo;
            break;
         case PAlarmDAT:
            cr->serNo = ((t_PAlarmMSG*)data_in)->serNo;
            break;
         case PTimeCalib:
            cr->serNo = ((t_PTimeCalib*)data_in)->serNo;
            break;
         default:
            ;// the incoming packet is not the one of 3, it is not an error
            /*res = P_RESPERR;
            fprintf(stderr, "\nERROR in packet_reply_former: Unknown pNo=0x%02X\n", ch_tmp->pNo);*/
      }
   } else {
      fprintf(stderr, "\nERROR: CRC error 0x%04X\n", crcCalc);
      res = P_ERRCRC;
   }
   
   if(res == P_NOERROR) {
      // still no error, CRC is the last one which need to be calculated
      //cr->crc = getCRC16((uInt8*)&cr->ch.len, cr->ch.len - sizeof(t_ErrChk));
      cr->crc = swapU16(getCRC16((uInt8*)&cr->ch.len, sizeof(t_PLen) + sizeof(t_PNo) + sizeof(t_InfoSerNo)));
   } else {
      fprintf(stderr, "\nERROR in packet_reply_former: %d\n", res);
   }
   
   return res;
}

void t_TimeLocation_printer(FILE* dev, int idx, t_TimeLocation* x) {
   int tmp;
   
   #ifdef DEBUG2
      fprintf(stderr, "\n|-----DEBUG START in t_TimeLocation_printer-----");
      fprintf(stderr, "\n| t_TimeLocation* x=\t%p", x);
      fprintf(stderr, "\n| x->lat bytes: ");
      printHEXandCHARS(stderr, (Int8*)(&(x->lat)), 0, sizeof(x->lat), FALSE);
      fprintf(stderr, "\n| x->lon bytes: ");
      printHEXandCHARS(stderr, (Int8*)(&(x->lon)), 0, sizeof(x->lon), FALSE);
      fprintf(stderr, "\n|-----DEBUG END in t_TimeLocation_printer  -----\n");
      fflush(stderr);
   #endif
   
   fprintf(dev, "%s\t", pNames[idx].pName);
   fprintf(dev, "%02d/%02d/%02d %02d:%02d:%02d\t", 
      (int)x->year, (int)x->month, (int)x->day,
      (int)x->hour, (int)x->min, (int)x->sec
   );
   fprintf(dev, "Sats\t%d\t", (int)x->sat);
   fprintf(dev, "Pos\t%.8f\t%.8f\t", 
      (float)swapU32(x->lat)/1.8e6, (float)swapU32(x->lon)/1.8e6
   );
   fprintf(dev, "Vel\t%4d\t", (int)x->speed);
   tmp = (int)swapU16(x->course);
   if( !((1<<5)&tmp) ) fprintf(dev, "\tReal time GPS\t");
   else fprintf(dev, "\tDiff. pos.\t");
   if( !((1<<4)&tmp) ) fprintf(dev, "\tGPS isn't posed.\t");
   else fprintf(dev, "\tGPS is posed.\t");
   if( !((1<<3)&tmp) ) fprintf(dev, "\tE-Long\t");
   else fprintf(dev, "\tW-Long\t");
   if( !((1<<2)&tmp) ) fprintf(dev, "\tS-Lat\t");
   else fprintf(dev, "\tN-Lat\t");
   fprintf(dev, "\tCourse\t%d\t", tmp&0x3FF);
}

void t_Pmob_Info_printer(FILE* dev, int idx, t_Pmob_Info* mob) {
   uInt32 tmp32;
   
   #ifdef DEBUG2
      fprintf(stderr, "\n|-----DEBUG START in t_Pmob_Info_printer-----");
      fprintf(stderr, "\n| t_Pmob_Info* mob=\t%p", mob);
      fprintf(stderr, "\n|-----DEBUG END in t_Pmob_Info_printer  -----\n");
   #endif
   
   tmp32 = 
      ((uInt32)mob->cellID[0] +
      ((uInt32)mob->cellID[1]<<8) +
      ((uInt32)mob->cellID[2]<<16) ) & 0x00FFFFFF;
   fprintf(dev, "\tMCC/MNC/LAC/CELLID\t%d/%d/%d/%u\t",
      (int)swapU16(mob->mcc), (int)mob->mnc,
      (int)swapU16(mob->lac), tmp32
   );
}

void t_SignalsAlarm_printer(FILE* dev, int idx, t_SignalsAlarm *sa) {
   int i;
   
   #ifdef DEBUG2
      fprintf(stderr, "\n|-----DEBUG START in t_SignalsAlarm_printer-----");
      fprintf(stderr, "\n| t_SignalsAlarm *sa=\t%p", sa);
      fprintf(stderr, "\n|-----DEBUG END in t_SignalsAlarm_printer  -----\n");
   #endif
   
   fprintf(dev, "%s\t", pNames[idx].pName);
   for(i = 0; i < HB_BITS; ++i)
      fprintf(dev, "%s%1d ", ((t_PHB*)pNames[idx].ic)[i].s,
         (int)((((t_PHB*)pNames[idx].ic)[i].v&sa->tic)==0?0:1)
      );
   fprintf(dev, "\tV-Lev(0-6)\t%1d", (int)sa->voltageLevel);
   fprintf(dev, "\tGSM-Lev(0-4)\t%1d", (int)sa->gsmLevel);
   // we skip language print out here
}

// all packets have the same structure
boolean checkCRC(char* data_in, int n_in, uInt16* crcCalc) {
   uInt16 crcPacket = swapU16(*((uInt16*)(data_in + n_in - sizeof(t_StopBit) - sizeof(t_ErrChk))));
   char* start = data_in + sizeof(t_StartBit);
   // the following might not work, search for t_StopBit is another alternative
   int n = n_in - (sizeof(t_StartBit) + sizeof(t_StopBit) + sizeof(t_ErrChk));
   
   *crcCalc = getCRC16((unsigned char*)start, n);
   
   #ifdef DEBUG
      fprintf(stderr, "\nDEBUG: checkCRC %d bytes: starting=%02X", n, (int)start[0]);
      fprintf(stderr, "\nDEBUG: crcPacket=%04X vs crcCalc=%04X\n", (uInt16)crcPacket, (uInt16)*crcCalc);
   #endif
   
   return ((*crcCalc==crcPacket) ? TRUE : FALSE);
}

//calculate the 16-bit CRC of data with predetermined length. 
uInt16 getCRC16(const unsigned char *pData, int nLength) { 
   uInt16 fcs = 0xffff; //initialization 
   while(nLength > 0){ 
      fcs = (fcs>>8)^crctab16[(fcs^*pData)&0xff]; 
      nLength--; 
      pData++; 
   } 
   return ~fcs; //negated 
} 

void printCheckTypes(FILE* dev) {
   uInt16 u16 = 0x1122;
   uInt32 u32 = 0x11223344;
   uInt64 u64 = 0x1122334455667788;
   size_t s_uInt16 = sizeof(uInt16), s_uInt32 = sizeof(uInt32),
      s_uInt64 = sizeof(uInt64), s_Int16 = sizeof(Int16),
      s_Int32 = sizeof(Int32), s_Int64 = sizeof(Int64);
   boolean err = FALSE;
   
   fprintf(dev, "\n############ Check endianness:");
   fprintf(dev, "\n\tCheck uInt16 0x%4X = ", u16);
   printHEXandCHARS(dev, (Int8*)&u16, 0, sizeof(uInt16), FALSE);
   fprintf(dev, "\n\tCheck uInt32 0x%8X = ", u32);
   printHEXandCHARS(dev, (Int8*)&u32, 0, sizeof(uInt32), FALSE);
   fprintf(dev, "\n\tCheck uInt64 0x%16llX = ", u64);
   printHEXandCHARS(dev, (Int8*)&u64, 0, sizeof(uInt64), FALSE);
   
   fprintf(dev, "\n############ Size of basic types:");
   fprintf(dev, "\n\tsizeof(%s) = %zu", "uInt8", sizeof(uInt8));
   fprintf(dev, "\n\tsizeof(%s) = %zu", "Int16", sizeof(Int16));
   fprintf(dev, "\n\tsizeof(%s) = %zu", "Int32", sizeof(Int32));
   fprintf(dev, "\n\tsizeof(%s) = %zu", "Int64", sizeof(Int64));
   fprintf(dev, "\n\tsizeof(%s) = %zu", "uInt16", sizeof(uInt16));
   fprintf(dev, "\n\tsizeof(%s) = %zu", "uInt32", sizeof(uInt32));
   fprintf(dev, "\n\tsizeof(%s) = %zu", "uInt64", sizeof(uInt64));
   
   fprintf(dev, "\n############ Check length of the basic types:");
   if(s_uInt16 != 2) {
      fprintf(dev, "\n\tWrong length of uInt16!");
      err = TRUE;
   }
   if(s_uInt32 != 4) {
      fprintf(dev, "\n\tWrong length of uInt32!");
      err = TRUE;
   }
   if(s_uInt64 != 8) {
      fprintf(dev, "\n\tWrong length of uInt64!");
      err = TRUE;
   }
   if(s_Int16 != 2) {
      fprintf(dev, "\n\tWrong length of Int16!");
      err = TRUE;
   }
   if(s_Int32 != 4) {
      fprintf(dev, "\n\tWrong length of Int32!");
      err = TRUE;
   }
   if(s_Int64 != 8) {
      fprintf(dev, "\n\tWrong length of Int64!");
      err = TRUE;
   }
   
   if(err) {
      fprintf(dev, "\nERROR: in bit length, please redefine types and recompile, exiting...");
      exit(1);
   }
   
   fprintf(dev, "\n\tsizeof(%s) = %zu", "t_StartBit", sizeof(t_StartBit));
   fprintf(dev, "\n\tsizeof(%s) = %zu", "t_PLen", sizeof(t_PLen));
   fprintf(dev, "\n\tsizeof(%s) = %zu", "t_PLen2", sizeof(t_PLen2));
   fprintf(dev, "\n\tsizeof(%s) = %zu", "t_PNo", sizeof(t_PNo));
   fprintf(dev, "\n\tsizeof(%s) = %zu", "t_InfoSerNo", sizeof(t_InfoSerNo));
   fprintf(dev, "\n\tsizeof(%s) = %zu", "t_ErrChk", sizeof(t_ErrChk));
   fprintf(dev, "\n\tsizeof(%s) = %zu", "t_StopBit", sizeof(t_StopBit));
   fprintf(dev, "\n\tsizeof(%s) = %zu", "t_IMEI", sizeof(t_IMEI));

   fprintf(dev, "\n############ Size of structures:");
   fprintf(dev, "\n\tsizeof(%s) = %zu", "t_PNames", sizeof(t_PNames));
   fprintf(dev, "\n\tsizeof(%s) = %zu", "t_PHB", sizeof(t_PHB));
   fprintf(dev, "\n\tsizeof(%s) = %zu", "t_PCommonHeader", sizeof(t_PCommonHeader));
   fprintf(dev, "\n\tsizeof(%s) = %zu", "t_PCommonHeader2", sizeof(t_PCommonHeader2));
   fprintf(dev, "\n\tsizeof(%s) = %zu", "t_PCommonRESP", sizeof(t_PCommonRESP));
   fprintf(dev, "\n\tsizeof(%s) = %zu", "t_PLoginMSG", sizeof(t_PLoginMSG));
   fprintf(dev, "\n\tsizeof(%s) = %zu", "t_PLoginRESP", sizeof(t_PLoginRESP));
   fprintf(dev, "\n\tsizeof(%s) = %zu", "t_Lang", sizeof(t_Lang));
   fprintf(dev, "\n\tsizeof(%s) = %zu", "t_TermInfoCont", sizeof(t_TermInfoCont));
   fprintf(dev, "\n\tsizeof(%s) = %zu", "t_SignalsAlarm", sizeof(t_SignalsAlarm));
   fprintf(dev, "\n\tsizeof(%s) = %zu", "t_PHeartBeatMSG", sizeof(t_PHeartBeatMSG));
   fprintf(dev, "\n\tsizeof(%s) = %zu", "t_PHeartBeatRESP", sizeof(t_PHeartBeatRESP));
   fprintf(dev, "\n\tsizeof(%s) = %zu", "t_TimeLocation", sizeof(t_TimeLocation));
   fprintf(dev, "\n\tsizeof(%s) = %zu", "t_Pmob_Info", sizeof(t_Pmob_Info));
   fprintf(dev, "\n\tsizeof(%s) = %zu", "t_PLocation", sizeof(t_PLocation));
   fprintf(dev, "\n\tsizeof(%s) = %zu", "t_PMileageDATA", sizeof(t_PMileageDATA));
   fprintf(dev, "\n\tsizeof(%s) = %zu", "t_PAlarmMSG", sizeof(t_PAlarmMSG));
   fprintf(dev, "\n\tsizeof(%s) = %zu", "t_PAlarmRESP", sizeof(t_PAlarmRESP));
   fprintf(dev, "\n\tsizeof(%s) = %zu", "t_PTimeCalib", sizeof(t_PTimeCalib));
   fprintf(dev, "\n\tsizeof(%s) = %zu", "t_PTimeCalibRESP", sizeof(t_PTimeCalibRESP));
   fprintf(dev, "\n\tsizeof(%s) = %zu", "t_POnlineCMD_InfoCONTENT", sizeof(t_POnlineCMD_InfoCONTENT));
   fprintf(dev, "\n\tsizeof(%s) = %zu", "t_POnlineCMD", sizeof(t_POnlineCMD));
   fprintf(dev, "\n\tsizeof(%s) = %zu", "t_POnlineREPLY_InfoCONTENT", sizeof(t_POnlineREPLY_InfoCONTENT));
   fprintf(dev, "\n\tsizeof(%s) = %zu", "t_POnlineREPLY", sizeof(t_POnlineREPLY));
   fprintf(dev, "\n\tsizeof(%s) = %zu", "t_PInfoTrans_InfoCONTENT", sizeof(t_PInfoTrans_InfoCONTENT));
   fprintf(dev, "\n\tsizeof(%s) = %zu", "t_PInfoTransMSG", sizeof(t_PInfoTransMSG));

}
