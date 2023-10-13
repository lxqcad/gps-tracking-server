# Copyright 2018-2019 by Juby Abraham
# The GPS Tracking Student. Emulate Infotech Pvt. Ltd.
# All rights reserved.
#Redistribution and use in source and binary forms, with or
#without modification, are permitted provided that the following 
#conditions are met:
#
# 1) Redistributions of source code must retain the above copyright
#notice, this list of conditions and the following disclaimer.
# 2) Redistributions in binary form must reproduce the above copyright
#notice, this list of conditions and the following disclaimer in the
#documentation and/or other materials provided with the distribution.
#
#THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND 
#ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
#IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR 
#PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS
#BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, 
#OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT 
#OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR 
#BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, 
#WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR 
#OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF 
#ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#
#The views and conclusions contained in the software and documentation are 
#those of the authors and should not be interpreted as representing official
#policies, either expressed or implied, of the GPS Track Server Project.
#

TR_SERVER = tr_srv
BIN_DIR = bin/
SRC_DIR = src/

# sources are here
SRC_SRV = $(SRC_DIR)/tr_server.c $(SRC_DIR)/tr_protocol.c $(SRC_DIR)/tr_utils.c

MKD = mkdir
CC = gcc
LD = ld

INCLUDES = -I./src/ -I/usr/local/include/ -I/usr/include/
LIB_PATH = -L/usr/local/lib/gcc48/ -L/usr/local/lib/ -L/usr/lib/ 

#DEBUG = -ggdb -g3 
DEBUG = -s

LDFLAGS += $(LDFLAGS_STATIC) -Ur $(DEBUG) $(LDFLAGS_GSL) -lm $(LIB_PATH) -lpthread

# optimized:
OPTIMIZED_FLAGS += -O3 -pipe

#.if defined (CPUTYPE) to be used for FreeBSD make
ifdef CPUTYPE
OPTIMIZED_FLAGS += -march=$(CPUTYPE)
endif

#-Wno-deprecated-declarations
CFLAGS += $(MODEL_COMMON_DEFS) $(DEBUG)  $(OPTIMIZED_FLAGS) $(INCLUDES) -Wall

CC_ALLFLAGS = $(CFLAGS) $(OPTIMIZED_FLAGS) $(LDFLAGS)

all:	clean $(TR_SERVER)

# compile server:
$(TR_SERVER):
	${MKD} -p ${BIN_DIR}
	$(CC) -o $(BIN_DIR)$(TR_SERVER) $(SRC_SRV) $(CC_ALLFLAGS)

clean:
	rm -f *.o core.* $(BIN_DIR)/$(TR_SERVER)

	
