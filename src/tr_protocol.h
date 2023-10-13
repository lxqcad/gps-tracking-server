/*
 * Copyright 2018-2019 by Juby Abraham
 * GPS Tracking Student. 
 * Emulate Infotech Pvt. Ltd.
 *
 */


#ifndef TR_PROTOCOL
#define TR_PROTOCOL

#include "tr_utils.h"

// byte padding is important for structures, otherwise type casting will not work
#pragma pack(1)

#define MAX_ALLOWED_LEN		1000

#define P_NOREPLY				-1		// do not send any response
#define P_NOERROR				0
#define P_UNKNOWN_TYPE		1
#define P_ERRCRC				2
#define P_ERRLEN				3
#define P_RESPERR				4

#define SUPPORTED_P	11
#define IMEI_LEN	8
#define StartBIT	(uInt16)0x7878
#define StopBIT	(uInt16)0x0D0A
#define StartBIT2	(uInt16)0x7979

// do not print phone numbers/IMEI for security reasons.
#define SWITCH_SECURE_PInfoTrans
//#define SWITCH_SECURE_IMEI

typedef uInt16		t_StartBit;
typedef uInt8		t_PLen;
typedef uInt16		t_PLen2;
typedef uInt8		t_PNo;
typedef uInt16		t_InfoSerNo;
typedef uInt16		t_ErrChk;
typedef uInt16		t_StopBit;
typedef uInt64		t_IMEI;

//the following is a complete list of packets
// the NR means server need to sent "reply" packet to terminal
// not all packets are implemented because they are not documented
typedef enum {
   PLogINFO = (t_PNo)0x01,		//NR, Login Information
   PPosDAT = (t_PNo)0x12,		//Positioning Data (UTC)
   PStatInfo = (t_PNo)0x13,	//NR, Status Info. (Heartbeat Packet)
   PAlarmDAT = (t_PNo)0x16,	//NR, Alarm Data (UTC) 
   PGPSAdrQ = (t_PNo)0x1A,		//GPS Address Query Packet 
   PLBSAdrQ = (t_PNo)0x17,		//LBS Address Query Packet 
   PLBSInfo = (t_PNo)0x18,		//LBS Multiple Base Station Extended Information Packet 
   PDemInfo = (t_PNo)0x80,		//Demand info. sent to terminal by server 
   PCMDResp = (t_PNo)0x15,		//Online cmd response by terminal
   PTimeCalib = (t_PNo)0x8A,	//Time Calibration Packet 
   PInfoTrans = (t_PNo)0x94,	//Information Transmission Packet
} t_ProtoNo;

typedef struct {
   t_PNo pNo;
   char pName[100];
   void* ic;
   size_t sz;
} t_PNames;

typedef struct {
   char v;
   char s[30];
} t_PHB;

// so far there only 2 types of "start bits" known
typedef struct {
   t_StartBit startBit;
   t_PLen len;
   t_PNo pNo;
} t_PCommonHeader;

typedef struct {
   t_StartBit startBit;
   t_PLen2 len;
   t_PNo pNo;
} t_PCommonHeader2;

// the following server->terminal response packet is applicable
//to at least 3 terminal messages: PLogINFO, PStatInfo and PAlarmDAT
typedef struct {
   t_PCommonHeader ch;
   t_InfoSerNo serNo;
   t_ErrChk crc;
   t_StopBit stopBit;
} t_PCommonRESP;

// Protocol number 0x01
typedef struct {
   t_PCommonHeader ch;
   uInt8 imei[IMEI_LEN];
   //t_IMEI imei;
   t_InfoSerNo serNo;
   t_ErrChk crc;
   t_StopBit stopBit;
} t_PLoginMSG;

typedef t_PCommonRESP t_PLoginRESP;

// terminal information:
enum {
   TermInfoCont_OilDisconnected = 128,
   TermInfoCont_GPSTrackingON = 64,
   TermInfoCont_ExtendedBits = 32 + 16 + 8,
   TermInfoCont_ChargeON = 4,
   TermInfoCont_ACCHigh = 2,
   TermInfoCont_DefenseActive = 1
};


typedef enum {
   t_Alarm_Normal = (uInt8)0x00,
   t_Alarm_SOS = (uInt8)0x01,
   t_Alarm_PowerCut = (uInt8)0x02,
   t_Alarm_Vibration = (uInt8)0x03,
   t_Alarm_EnterFence = (uInt8)0x04,
   t_Alarm_ExitFence = (uInt8)0x05,
   t_Alarm_OverSpeed = (uInt8)0x06,
   t_Alarm_Displacement = (uInt8)0x09,
   t_Alarm_EnterGPSDeadZone = (uInt8)0x0A,
   t_Alarm_ExitGPSDeadZone = (uInt8)0x0B,
   t_Alarm_PowerOn = (uInt8)0x0C,
   t_Alarm_GPS1stFix = (uInt8)0x0D,
   t_Alarm_LowBattery = (uInt8)0x0E,
   t_Alarm_LowBatteryProtection = (uInt8)0x0F,
   t_Alarm_SIMChangeNotice = (uInt8)0x10,
   t_Alarm_PowerOff = (uInt8)0x11,
   t_Alarm_AirplaneMode = (uInt8)0x12,
   t_Alarm_Disassemble = (uInt8)0x13,
   t_Alarm_Door = (uInt8)0x14,
   t_Alarm_ACCOn = (uInt8)0xFE,
   t_Alarm_ACCOff = (uInt8)0xFF
} t_Alarm;

typedef enum {
   t_Lang_Chinese = (uInt8)0x01,
   t_Lang_English = (uInt8)0x02
} t_Lang;

// Protocol number 0x13
typedef uInt8 t_TermInfoCont;

typedef struct {
   t_TermInfoCont tic;
   uInt8 voltageLevel;
   uInt8 gsmLevel;
   t_Alarm alarm; // for 0x16, it might be not the same structure for 0x13
   t_Lang lang;
} t_SignalsAlarm;

typedef struct {
   t_PCommonHeader ch;
   t_SignalsAlarm infoCont;
   t_InfoSerNo serNo;
   t_ErrChk crc;
   t_StopBit stopBit;
} t_PHeartBeatMSG;

typedef t_PCommonRESP t_PHeartBeatRESP;

// Protocol number 0x12
typedef struct {
   uInt8 year;
   uInt8 month;
   uInt8 day;
   uInt8 hour;
   uInt8 min;
   uInt8 sec;
   uInt8 sat;  // the format of this byte is unclear
   uInt32 lat; // need to be div. by 1800000
   uInt32 lon; // need to be div. by 1800000
   uInt8 speed;
   uInt16 course; // 0..360deg, clockwise from North 0deg, TBD!!!
} t_TimeLocation;

typedef struct {
   uInt16 mcc;
   uInt8 mnc;
   uInt16 lac;
   uInt8 cellID[3];
} t_Pmob_Info;

typedef struct {
   t_PCommonHeader ch;
   t_TimeLocation x;
   t_Pmob_Info mob;
   t_InfoSerNo serNo;
   t_ErrChk crc;
   t_StopBit stopBit;
} t_PLocation;

typedef struct {
   t_PCommonHeader ch;
   t_TimeLocation x;
   t_Pmob_Info mob;
   uInt32 mileage;
   t_InfoSerNo serNo;
   t_ErrChk crc;
   t_StopBit stopBit;
} t_PMileageDATA;

// Protocol number 0x16
typedef struct {
   t_PCommonHeader ch;
   t_TimeLocation x;
   uInt8 lbs_len;
   t_Pmob_Info mob;
   t_SignalsAlarm alarmCont;
   t_InfoSerNo serNo;
   t_ErrChk crc;
   t_StopBit stopBit;
} t_PAlarmMSG;

typedef t_PCommonRESP t_PAlarmRESP;

// following is not documented, it is unknown if server must respond
// Protocol 0x8A
// 78 78 05 8A 00 02 CE 0D 0D 0A 00 00 00 00 00 00 
typedef struct {
   t_PCommonHeader ch;
   t_InfoSerNo serNo;
   t_ErrChk crc;
   t_StopBit stopBit;
} t_PTimeCalib;

typedef t_PCommonRESP t_PTimeCalibRESP;

// the following is not copyable because of pointer!
// Protocol number 0x80
typedef struct {
   uInt8 len;
   uInt32 flags;
   char *cmd;
} t_POnlineCMD_InfoCONTENT;

typedef struct {
   t_PCommonHeader ch;
   t_POnlineCMD_InfoCONTENT infoCont;
   t_InfoSerNo serNo;
   t_ErrChk crc;
   t_StopBit stopBit;
} t_POnlineCMD;

// the following is not copyable because of pointer!
// Protocol number 0x15
typedef struct {
   uInt8 len;
   uInt32 flags;
   char *cmd;
   uInt8 unsed; // part of language
   t_Lang lang;
} t_POnlineREPLY_InfoCONTENT;

typedef struct {
   t_PCommonHeader ch;
   t_POnlineREPLY_InfoCONTENT infoCont;
   t_InfoSerNo serNo;
   t_ErrChk crc;
   t_StopBit stopBit;
} t_POnlineREPLY;


// the following is not copyable because of pointer!
// Protocol number 0x94
// the following is totally unclear from the Chinese document
// Seem this is special packet:
//		- StartBIT2
//		- Packet length is 2 bytes, not 1
//		- Packet length is unclear, thus we make a guess.
#define SIZE_t_PInfoTrans_InfoCONTENT (154-(sizeof(uInt8)+sizeof(t_PCommonHeader2)+sizeof(t_InfoSerNo)+sizeof(t_ErrChk)+sizeof(t_StopBit)) )
typedef struct {
   uInt8 subP;
   char unknown_or_variable[SIZE_t_PInfoTrans_InfoCONTENT];
} t_PInfoTrans_InfoCONTENT;

typedef struct {
   t_PCommonHeader2 ch;
   t_PInfoTrans_InfoCONTENT infoCont;
   t_InfoSerNo serNo;
   t_ErrChk crc;
   t_StopBit stopBit;
} t_PInfoTransMSG;

uInt16 getCRC16(const uInt8 *pData, int nLength);
int packet_handler(char* data_in, int n_in, char* data_out, int* n_out, char* imei);
int packet_reply_former(char* data_in, int n_in, char* data_out, int* n_out, char* imei);
void packetLogger(FILE* dev, char* data_in, int n_in, char* imei);
boolean checkCRC(char* data_in, int n_in, uInt16* crcCalc);
void t_SignalsAlarm_printer(FILE* dev, int idx, t_SignalsAlarm *sa);
void t_TimeLocation_printer(FILE* dev, int idx, t_TimeLocation* loc);
void t_Pmob_Info_printer(FILE* dev, int idx, t_Pmob_Info* mob);
void printCheckTypes(FILE* dev);

#endif /* TR_PROTOCOL */

