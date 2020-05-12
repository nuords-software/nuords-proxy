#********************************************************************
# $Id:$
# $Revision: 1.2 $
# ********************************************************************
# Project : NuoRDS Proxy
# 
# ********************************************************************
# Description :  Makefile for NuoRDS Proxy (macOS). 
# 
# ********************************************************************

#Config

ifeq ($(OS_SYS),)
	OS_SYS:=mac
endif

BLD_ARCH:=-arch x86_64
CC_OPT:=-mmacosx-version-min=10.12

#Libraries

LK_LIB:=-lpthread

include ../unix/common.mak

SHIP_DAT:=$(BLD_DAT)
SHIP_DIR:=../ship/$(OS_SYS)/$(PROD_NAME)/$(BLD_HOST)/$(OS_VER)$(ALT_ARCH)
SHIP_OUT:=../ship/out/$(PROD_NAME)/$(BLD_HOST)
SHIP_ZIP:=$(SHIP_OUT)/nrdproxy-$(OS_VER)$(ALT_ARCH)-$(SHIP_DAT).zip

ship: nrdproxy
	-@echo "Shipping..."
	-@if [ -f "$(SHIP_ZIP)" ] ; then rm -f "$(SHIP_ZIP)" ; fi
	-@if [ -d "$(SHIP_DIR)" ] ; then rm -f "$(SHIP_DIR)"/* ; fi
	-@if [ ! -d "$(SHIP_DIR)" ] ; then $(MD_CMD) "$(SHIP_DIR)" ; fi
	-@if [ -d "$(SHIP_DIR)" ] ; then echo "Ship dir: "$(SHIP_DIR) ; fi
	-@if [ ! -d "$(SHIP_OUT)" ] ; then $(MD_CMD) "$(SHIP_OUT)" ; fi
	-@if [ -f "$(SHIP_ZIP)" ] ; then rm -f "$(SHIP_ZIP)" ; fi 
	-@cp -p "$(BIN_OUT)" "$(SHIP_DIR)/"
	-@perl -p -e 's/\r\n/\n/' "setup/com.nuords.nrdproxyd.plist" > "$(SHIP_DIR)/com.nuords.nrdproxyd.plist"
	-@perl -p -e 's/\r\n/\n/' "setup/nrdproxy_mac_install" > "$(SHIP_DIR)/nrdproxy_mac_install"
	-@perl -p -e 's/\r\n/\n/' "setup/nrdproxy_mac_uninstall" >  "$(SHIP_DIR)/nrdproxy_mac_uninstall"
	-@perl -p -e 's/\r\n/\n/' "setup/nrdproxyctl" >  "$(SHIP_DIR)/nrdproxyctl"
	-@perl -p -e 's/\r\n/\n/' "../../doc/INSTALL.md" > "$(SHIP_DIR)/INSTALL.md"
	-@perl -p -e 's/\r\n/\n/' "../../README.md" > "$(SHIP_DIR)/README.md"
	-@perl -p -e 's/\r\n/\n/' "../conf/nrdproxyd.cfg.template" > "$(SHIP_DIR)/nrdproxyd.cfg.template"
	-@PWD=`pwd`; cd "$(SHIP_DIR)/" ; zip "$(PWD)/$(SHIP_ZIP)" *
	-@if [ -f "$(SHIP_ZIP)" ] ; then echo "Ship zip: "$(SHIP_ZIP) ; fi  
.PHONY: ship
