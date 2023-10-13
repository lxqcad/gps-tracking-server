/*
 *
 * Copyright 2018-2019 by Juby Abraham
 * GPS Tracking Student. 
 * Emulate Infotech Pvt. Ltd.
 *
 */
 
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h> 
#include <time.h> 
#include "tr_server.h"
#include "tr_protocol.h"
#include "tr_utils.h"

static uInt16 tr_port = DEFUALT_TCP_PORT;

// whole statistics is kept there
static t_Stat stats;

int main(int argc, char** argv) {
   int res = 0;
   int socket_desc, client_sock, c;
   struct sockaddr_in server, client;
   pthread_t thread_id, periodic_id;
   
   fprintf(stderr, "\n%s\nVersion: %s\n", COPYRIGHT, VERSION);
   printCheckTypes(stderr);
   argumentsHandler(argc, argv);
   
   if( pthread_create( &periodic_id, NULL, periodic, (void*) &client_sock) < 0) {
      fprintf(stderr, "\nERROR: could not create periodic thread");
      return 1;
   }
   
   //Create socket
   socket_desc = socket(AF_INET, SOCK_STREAM, 0);
   if (socket_desc == -1) {
      fprintf(stderr, "\nERROR: Could not create socket.");
      return 1;
   }
   fprintf(stderr, "\nINFO: Socket created");
    
   //Prepare the sockaddr_in structure
   server.sin_family = AF_INET;
   server.sin_addr.s_addr = INADDR_ANY;
   server.sin_port = htons(tr_port);
    
   //Bind
   if( bind(socket_desc,(struct sockaddr *)&server, sizeof(server)) < 0) {
      //print the error message
      fprintf(stderr, "\nERROR: Bind failed.\n\n");
      return 1;
   }
   fprintf(stderr, "\nINFO: bind done");
   
   //Listen
   listen(socket_desc, 3);
   
   //Accept and incoming connection
   fprintf(stderr, "\nINFO: Waiting for incoming connections...");
   c = sizeof(struct sockaddr_in);

   while( (client_sock = accept(socket_desc, (struct sockaddr *)&client, (socklen_t*)&c)) ) {
      fprintf(stderr, "\nINFO: Connection accepted");
       
      if( pthread_create( &thread_id, NULL, terminal_handler, (void*) &client_sock) < 0) {
         fprintf(stderr, "\nERROR: could not create thread");
         res = 1;
         break;
      } else {
         //Now join the thread, so that we dont terminate before the thread
         //pthread_join( thread_id, NULL);
         fprintf(stderr, "\nINFO: Handler assigned");
      }
   }
   
   if (client_sock < 0) {
      fprintf(stderr, "\nERROR: accept failed");
      res = 1;
   }
   return res;
}
 
/*
 * This will handle connection for each client
 * */
void* terminal_handler(void *socket_desc) {
   //Get the socket descriptor
   int sock = *(int*)socket_desc, err;
   int read_size, send_size;
   char msg_in[MAX_ALLOWED_LEN], msg_out[MAX_ALLOWED_LEN];
   char imei[50]; // this is text representation of imei
    
   // fill imei with zeros
   memset(imei, 0, 50);
   
   //Receive a message from client
   while( (read_size = recv(sock, msg_in, MAX_ALLOWED_LEN, 0)) > 0 ) {
      stats.bytesIn += (uInt64)read_size;
      
      err = packet_handler(msg_in, read_size, msg_out, &send_size, imei);
      if(err == P_NOERROR) {
         //Send the message back to client
         write(sock, msg_out, send_size);
         stats.bytesOut += (uInt64)send_size;
         
         fprintf(stderr, "\nINFO: Reply sent to client!\n");
         #ifdef DEBUG
            fprintf(stderr, "\n|-----DEBUG START in terminal_handler-----");
            fprintf(stderr, "\nDEBUG: Sent %d bytes:\n", send_size);
            printHexBytes(stderr, msg_out, (send_size<MAX_ALLOWED_LEN)?send_size:MAX_ALLOWED_LEN);
            fprintf(stderr, "|-----DEBUG END in terminal_handler  -----\n");
         #endif
      } else switch(err) {
         case P_NOREPLY:
            fprintf(stderr, "\nINFO: no response needed");
            break;
         default:
            fprintf(stderr, "\nERROR: packet_handler returned err = %d, no response is sent to terminal.", err);
      }
      //clear message buffers
      memset(msg_in, 0, MAX_ALLOWED_LEN);
      memset(msg_out, 0, MAX_ALLOWED_LEN);
   }
    
   if(read_size == 0) {
      fprintf(stderr, "\nClient disconnected");
   } else if(read_size == -1) {
      fprintf(stderr, "\nERROR: recv failed");
   }
   fflush(stdout);
   fflush(stderr);
   return NULL;
}

void* periodic(void *x) {
   while(1) {
      conditionalSleep();
      updateStatistics(stdout);
      fflush(stdout);
      fflush(stderr);
   }
   return NULL;
}

// here we change periodic delay depending if statistics has been 
//updated or not
static void conditionalSleep() {
   static t_Stat st_copy;
   
   // check if incoming data differ:
   if( st_copy.bytesIn != stats.bytesIn ) {
      // data differ
      memcpy(&st_copy, &stats, sizeof(t_Stat));
      // short sleep pause
      sleep(PERIODIC_CALL_MIN_SEC);
   } else {
      // data are identical
      while( ( st_copy.bytesIn == stats.bytesIn ) &&
         (time(NULL) - stats.curr_time) < PERIODIC_CALL_MAX_SEC
      ) {
         // long sleep pause
         fflush(stdout);
         sleep(PERIODIC_CALL_MIN_SEC);
      }
   }
}

static void updateStatistics(FILE *dev) {
   static uInt64 call_counter = 0;
   float tmp;
   char tstr[50];
   
   if(!call_counter) {
      // the 1st call: initialization
      stats.start_time = time(NULL);
      stats.bytesIn = 0;
      stats.bytesOut = 0;
      ++call_counter;
   } else {
      // the rest of the calls: updating
      ++call_counter;
      stats.curr_time = time(NULL);
      tmp = (3600.0 / 1024.0) / (stats.curr_time - stats.start_time);
      stats.kbPerHourIn = stats.bytesIn * tmp;
      stats.kbPerHourOut = stats.bytesOut * tmp;
      stats.kbPerHourTot = stats.kbPerHourIn + stats.kbPerHourOut;
      fprintf(dev, "\n%s TRAFF%6.1f kB/h %5.1f Mb/mon (TOTAL)%6.1f kB/h (IN)%6.1f kB/h (OUT)%7.1f hours (IDLE) %.1e/%.1e (bytes IN/OUT) %llu (CALLS)",
         time2str(tstr), 
         stats.kbPerHourTot, 
         (720.0 / 1024.0) * stats.kbPerHourTot, 
         stats.kbPerHourIn, 
         stats.kbPerHourOut, 
         1.0 / (tmp * 1024.0),
         (double)stats.bytesIn, 
         (double)stats.bytesOut,
         call_counter
      );
      fflush(dev);
   }
}

inline void argumentsHandler(int argc, char** argv) {
   uInt32 p;
   
   switch(argc) {
      case 1:
         fprintf(stderr, "\n\nINFO: No port number is specified, using default %d\n", tr_port);
         break;
      case 2:
         p = atol(argv[1]);
         if(p > 65535) {
            fprintf(stderr, "\n\nERROR: Specified port number (%u) is invalid, using default %d\n\tsleeping 10sec...\n", p, tr_port);
            p = 10;
            while(p--) {
               fprintf(stderr, "."); fflush(stderr);
               sleep(1);
            }
         } else {
            tr_port = (uInt16)p;
            fprintf(stderr, "\n\nINFO: Using specified port number %d\n", tr_port);
         }
         break;
      default:
         fprintf(stderr, "\n\nWARNING: Too many arguments, ignoring, using default port %d\n\tsleeping 10sec...\n", tr_port);
         p = 10;
         while(p--) {
            fprintf(stderr, "."); fflush(stderr);
            sleep(1);
         }
   }
}