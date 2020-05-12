#********************************************************************
# $Id:$
# $Revision: 1.2 $
# ********************************************************************
# Project : NuoRDS Proxy
# 
# ********************************************************************
# Description :  Makefile for NuoRDS Proxy (Unix). 
# 
# ********************************************************************

#Config

ifeq ($(OS_SYS),)
	OS_SYS:=unix
endif

ifeq ($(BLD_ARCH),)
	BLD_ARCH:=-m64
endif 

#Libraries

LK_LIB:=-pthread

#Includes

include ../unix/common.mak

SHIP_DAT:=$(BLD_DAT)
SHIP_DIR:=../ship/$(OS_SYS)/$(BLD_HOST)/$(OS_VER)$(ALT_ARCH)
SHIP_PAC:=../ship/pack/$(BLD_HOST)
SHIP_ZIP:=$(SHIP_PAC)/$(PROD_NAME)-$(OS_VER)$(ALT_ARCH)-$(SHIP_DAT).zip

ship: nrdproxy
	-@echo "Shipping..."$(OS_SYS)
	-@if [ -f "$(SHIP_ZIP)" ] ; then rm -f "$(SHIP_ZIP)" ; fi
	-@if [ -d "$(SHIP_DIR)" ] ; then rm -f "$(SHIP_DIR)"/* ; fi
	-@if [ ! -d "$(SHIP_DIR)" ] ; then $(MD_CMD) "$(SHIP_DIR)" ; fi
	-@if [ -d "$(SHIP_DIR)" ] ; then echo "Ship dir: "$(SHIP_DIR) ; fi
	-@if [ ! -d "$(SHIP_PAC)" ] ; then $(MD_CMD) "$(SHIP_PAC)" ; fi
	-@if [ -f "$(SHIP_ZIP)" ] ; then rm -f "$(SHIP_ZIP)" ; fi 
	-@cp -p "$(BIN_OUT)" "$(SHIP_DIR)/"
	-@#-@perl -p -e 's/\r\n/\n/' "setup/nrdproxy_mac_install" > "$(SHIP_DIR)/nrdproxy_uni_install"
	-@#-@perl -p -e 's/\r\n/\n/' "setup/nrdproxy_mac_uninstall" >  "$(SHIP_DIR)/nrdproxy_uni_uninstall"
	-@#-@perl -p -e 's/\r\n/\n/' "setup/nrdproxyctl" >  "$(SHIP_DIR)/nrdproxyctl"
	-@perl -p -e 's/\r\n/\n/' "../../doc/INSTALL.md" > "$(SHIP_DIR)/INSTALL.md"
	-@perl -p -e 's/\r\n/\n/' "../../README.md" > "$(SHIP_DIR)/README.md"
	-@perl -p -e 's/\r\n/\n/' "../conf/nrdproxyd.cfg.template" > "$(SHIP_DIR)/nrdproxyd.cfg.template"
	-@PWD=`pwd`; cd "$(SHIP_DIR)/" ; zip "$(PWD)/$(SHIP_ZIP)" *
	-@if [ -f "$(SHIP_ZIP)" ] ; then echo "Ship zip: "$(SHIP_ZIP) ; fi 

.PHONY: ship
