# ********************************************************************
# Project : NuoRDS Proxy
# 
# ********************************************************************
# Description :  Makefile for NuoRDS Proxy (Unix). 
# 
# ********************************************************************

#Config

ifeq ($(BLD_DEST),)
	BLD_DEST:=unix
endif

#Libraries

LNK_LIBS:=-pthread

include ../unix/common.mak

SUB_PATH:=$(BLD_DEST)/$(BLD_CONF)
SHIP_DIR:=../ship/$(SUB_PATH)
PACK_DIR:=../pack/$(SUB_PATH)
PACK_ZIP:=$(PACK_DIR)/$(PROD_NAME)-$(BLD_DEST)-$(BLD_DATE).zip

ship: proxy
	-@echo "Shipping..."
	-@if [ -f "$(PACK_ZIP)" ] ; then rm -f "$(PACK_ZIP)" ; fi
	-@if [ -d "$(SHIP_DIR)" ] ; then rm -f "$(SHIP_DIR)"/* ; fi
	-@if [ ! -d "$(SHIP_DIR)" ] ; then $(MKDIR_CMD) "$(SHIP_DIR)" ; fi
	-@if [ -d "$(SHIP_DIR)" ] ; then echo "Ship dir: "$(SHIP_DIR) ; fi
	-@cp -p "$(BIN_OUT)" "$(SHIP_DIR)/"
	-@perl -p -e 's/\r\n/\n/' "../../doc/README.md" > "$(SHIP_DIR)/README.md"
	-@perl -p -e 's/\r\n/\n/' "../../doc/INSTALL.md" > "$(SHIP_DIR)/INSTALL.md"
	-@perl -p -e 's/\r\n/\n/' "../../doc/EXAMPLES.md" > "$(SHIP_DIR)/EXAMPLES.md"
	-@perl -p -e 's/\r\n/\n/' "../../doc/CONFIG.md" > "$(SHIP_DIR)/CONFIG.md"
	-@perl -p -e 's/\r\n/\n/' "../../doc/ADDRESS.md" > "$(SHIP_DIR)/ADDRESS.md"
	-@perl -p -e 's/\r\n/\n/' "../conf/nrdproxyd.cfg.sample" > "$(SHIP_DIR)/nrdproxyd.cfg.sample"
	-@perl -p -e 's/\r\n/\n/' "../conf/nrdproxyd.cfg.simple" > "$(SHIP_DIR)/nrdproxyd.cfg.simple"

pack:
	-@echo "Packing..."
	-@if [ -f "$(PACK_ZIP)" ] ; then rm -f "$(PACK_ZIP)" ; fi
	-@if [ ! -d "$(PACK_DIR)" ] ; then $(MKDIR_CMD) "$(PACK_DIR)" ; fi 
	-@PWD=`pwd`; cd "$(SHIP_DIR)/" ; zip "$(PWD)/$(PACK_ZIP)" *
	-@if [ -f "$(PACK_ZIP)" ] ; then echo "Pack zip: "$(PACK_ZIP) ; fi
	
.PHONY: ship pack
