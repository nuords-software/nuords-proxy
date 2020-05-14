# ********************************************************************
# Project : NuoRDS Proxy
# 
# ********************************************************************
# Description :  Common Makefile for NuoRDS Proxy.  
# 
# ********************************************************************

#BUILD OPTIONS

#Architecture name
ifeq ($(BLD_DEST),)
	BLD_DEST:=default
endif

#Product name
ifeq ($(PROD_NAME),)
	PROD_NAME:=nrdproxy
endif

#Binary name
ifeq ($(BIN_NAME),)
	BIN_NAME:=nrdproxyd
endif

#Build config
ifeq ($(BLD_CONF),)
	BLD_CONF:=release
endif

#System name
ifeq ($(OS_NAME),)
	OS_NAME:=other
endif

ifeq ($(LNK_LIBS),)
  LNK_LIBS:=-lpthread
endif  

#Target system version
ifeq ($(OS_VER),)
  DUMMY:=$(shell perl -p -e 's/\r\n/\n/' ../conf/config.guess > ../conf/config.guess.tmp)
	OS_VER:=$(shell echo `sh ../conf/config.guess.tmp`)
	DUMMY:=$(shell rm -f ../conf/config.guess.tmp)
endif

ifeq ($(OS_VER),)
	OS_VER:=unknown
endif

BLD_DATE=$(shell date "+%Y%m%d")

ifeq ($(BLD_DATE),)
	BLD_DATE:=000000
endif 

#Compiler options
CPP_OPTS+=$(BLD_OPTS) -Wno-deprecated -Wno-unused-result

#Compiler flags
ifeq ($(BLD_CONF),debug)
	CPP_FLAGS+=-g -D_DEBUG
else
	CPP_FLAGS+=-O2
endif

#Compiler command
CPP_CMD:=g++ $(CPP_OPTS)

#Make dir command
ifeq ($(MKDIR_CMD),)
	MKDIR_CMD:=mkdir -p
endif

#Paths
BLD_DIR=../bld
XTL_DIR=../com/xtllib
INC_DIR=../com/include

OBJ_DIR=$(BLD_DIR)/$(OS_NAME)/$(OS_VER)/$(BLD_DEST)/$(BLD_CONF)/obj
BIN_DIR=$(BLD_DIR)/$(OS_NAME)/$(OS_VER)/$(BLD_DEST)/$(BLD_CONF)/bin
BIN_OUT=$(BIN_DIR)/$(BIN_NAME)

#Linker objects and includes
LNK_OBJS=nrdnb_client.o nrdpx_config.o nrdpx_server.o nrdpx_socket.o
CPP_INCL=-I. -I.. -I$(XTL_DIR) -I$(INC_DIR)

all: proxy

vers:
	-@perl -p -e 's/\r\n/\n/' "../unix/version.sh" >  "version.tmp.sh"
	-@chmod +x ./version.tmp.sh ; ./version.tmp.sh ../nrdpx_version.h
	-@rm -f ./version.tmp.sh

dirs:
	-@echo "Pwd dir: "`pwd`
	-@echo "Bin dir: "$(BIN_DIR)
	-@echo "Obj dir: "$(OBJ_DIR)
	-@if [ ! -d "$(BIN_DIR)" ] ; then $(MKDIR_CMD) "$(BIN_DIR)" ; fi
	-@if [ ! -d "$(OBJ_DIR)" ] ; then $(MKDIR_CMD) "$(OBJ_DIR)" ; fi

proxy: vers dirs objs
	-@PWD=`pwd`; cd "$(OBJ_DIR)" ; $(CPP_CMD) $(CPP_FLAGS) $(LNK_LIBS) -o "$(PWD)/$(BIN_OUT)" $(LNK_OBJS) 

objs: $(LNK_OBJS)

nrdnb%.o:
	$(CPP_CMD) -c $(CPP_FLAGS) $(CPP_INCL) -o "$(OBJ_DIR)/$@"  ../nrdnb$*.cpp
	
nrdpx%.o:
	$(CPP_CMD) -c $(CPP_FLAGS) $(CPP_INCL) -o "$(OBJ_DIR)/$@"  ../nrdpx$*.cpp

%.o:
	$(CPP_CMD) -c $(CPP_FLAGS) $(CPP_INCL) -o "$(OBJ_DIR)"/$@  $*.cpp 

clean:
	-@rm -f  $(BIN_OUT)
	-@rm -f  $(OBJ_DIR)/*.o  
	
.PHONY: all vers dirs proxy objs clean 
