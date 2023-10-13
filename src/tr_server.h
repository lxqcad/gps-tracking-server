/*
 * Copyright 2018-2019 by Juby Abraham
 * GPS Tracking Student. 
 * Emulate Infotech Pvt. Ltd.
 *
 */

#ifndef TR_SERVER
#define TR_SERVER

#include <time.h> 
#include "tr_utils.h"

#define COPYRIGHT			"GPS Track Server, (C) Juby Abraham, Emulate Infotech Pvt. Ltd."
#define VERSION			"v1.077"
#define DEFUALT_TCP_PORT		0x7472U
#define PERIODIC_CALL_MIN_SEC	3600U
#define PERIODIC_CALL_MAX_SEC	43200U

typedef struct {
   uInt64 bytesIn;
   uInt64 bytesOut;
   float kbPerHourIn;
   float kbPerHourOut;
   float kbPerHourTot;
   time_t start_time;
   time_t curr_time;
} t_Stat;

void argumentsHandler(int argc, char** argv);
void* terminal_handler(void *socket_desc);
static void updateStatistics(FILE *dev);
void* periodic(void *x);
static void conditionalSleep();

#endif /* TR_SERVER */

