#********************************************************************
# $Id: $
# $Revision: 1.2 $
# ********************************************************************
# Project : NuoRDS Proxy
# 
# ********************************************************************
# Description :  Common Makefile for NuoRDS Proxy (Unix/Linux).  
# 
# ********************************************************************

#ALTERNATIVE BUILD ARCHITECTURE

#ALT_ARCH:=-m32
#ALT_ARCH:=-m64

#BUILD OPTIONS

ifneq ($(ALT_ARCH),)
	BLD_ARCH:=$(ALT_ARCH)
endif

#Binary name
ifeq ($(PROD_NAME),)
	PROD_NAME:=nrdproxy
endif

ifeq ($(BIN_NAME),)
	BIN_NAME:=nrdproxyd
endif

#Output config
ifeq ($(BLD_CFG),)
	BLD_CFG:=release
endif

#Target host name
ifeq ($(BLD_HOST),)
	BLD_HOST:=$(shell uname -n)
endif

ifeq ($(BLD_HOST),)
	BLD_HOST:=unknown
endif

#Common system identifier
ifeq ($(OS_SYS),)
	OS_SYS:=other
endif

ifeq ($(LK_LIB),)
  LK_LIB:=-lpthread
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

BLD_DAT=$(shell date "+%Y%m%d")

ifeq ($(BLD_DAT),)
	BLD_DAT:=000000
endif 

#Compiler options and flags
CC_OPT+=$(BLD_ARCH) -Wno-deprecated -Wno-unused-result

ifeq ($(BLD_CFG),debug)
	CC_FLG+=-g -D_DEBUG
else
	CC_FLG+=-O2
endif

#C++ compiler
CC_CMD:=g++ $(CC_OPT)

#Make dir command
ifeq ($(MD_CMD),)
	MD_CMD:=mkdir -p
endif

#Paths
BLD_DIR=../bld
XTL_DIR=../com/xtllib
INC_DIR=../com/include

OBJ_DIR=$(BLD_DIR)/$(OS_SYS)/$(BLD_HOST)/$(OS_VER)$(ALT_ARCH)/$(PROD_NAME)/$(BLD_CFG)/obj
BIN_DIR=$(BLD_DIR)/$(OS_SYS)/$(BLD_HOST)/$(OS_VER)$(ALT_ARCH)/$(PROD_NAME)/$(BLD_CFG)/bin
BIN_OUT=$(BIN_DIR)/$(BIN_NAME)

#Linker objects and includes
LK_OBJ=nrdnb_client.o nrdpx_config.o nrdpx_server.o nrdpx_socket.o
CC_INC=-I. -I.. -I$(XTL_DIR) -I$(INC_DIR)

all: nrdproxy

version:
	-@perl -p -e 's/\r\n/\n/' "../unix/version.sh" >  "version.tmp.sh"
	-@chmod +x ./version.tmp.sh ; ./version.tmp.sh ../nrdpx_version.h
	-@rm -f ./version.tmp.sh

dirs:
	-@echo "Pwd dir: "`pwd`
	-@echo "Bin dir: "$(BIN_DIR)
	-@echo "Obj dir: "$(OBJ_DIR)
	-@if [ ! -d "$(BIN_DIR)" ] ; then $(MD_CMD) "$(BIN_DIR)" ; fi
	-@if [ ! -d "$(OBJ_DIR)" ] ; then $(MD_CMD) "$(OBJ_DIR)" ; fi
	
conf:
	-@cp ../conf/nrdproxyd.cfg.template      "$(BIN_DIR)"/
	-@cp ../conf/nrdproxyd.cfg               "$(BIN_DIR)"/
      
nrdproxy: version dirs objects
	-@PWD=`pwd`; cd "$(OBJ_DIR)" ; $(CC_CMD) $(CC_FLG) $(LK_LIB) -o "$(PWD)/$(BIN_OUT)" $(LK_OBJ) 

objects: $(LK_OBJ)

nrdnb%.o:
	$(CC_CMD) -c $(CC_FLG) $(CC_INC) -o "$(OBJ_DIR)/$@"  ../nrdnb$*.cpp
	
nrdpx%.o:
	$(CC_CMD) -c $(CC_FLG) $(CC_INC) -o "$(OBJ_DIR)/$@"  ../nrdpx$*.cpp

%.o:
	$(CC_CMD) -c $(CC_FLG) $(CC_INC) -o "$(OBJ_DIR)"/$@  $*.cpp 

clean:
	-@rm -f  $(BIN_OUT)
	-@rm -f  $(OBJ_DIR)/*.o  
	
.PHONY: objects clean all nrdproxy
