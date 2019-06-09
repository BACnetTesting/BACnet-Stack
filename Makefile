# Main Makefile for BACnet-stack project with GCC

# tools - only if you need them.
# Most platforms have this already defined
# CC = g++
# AR = ar
# MAKE = make
# SIZE = size
#
# Assumes rm and cp are available

#DEBUGMAKE=/dev/tty1
DEBUGMAKE=/dev/stderr

# configuration
# If BACNET_DEFINES has not already been set, configure to your needs here
MY_BACNET_DEFINES = -DPRINT_ENABLED=1
MY_BACNET_DEFINES += -DBACAPP_ALL
MY_BACNET_DEFINES += -DBACFILE=0
MY_BACNET_DEFINES += -DINTRINSIC_REPORTING=0
MY_BACNET_DEFINES += -DBACNET_TIME_MASTER=0
MY_BACNET_DEFINES += -DBACNET_PROPERTY_LISTS=1
MY_BACNET_DEFINES += -DBACNET_PROTOCOL_REVISION=17
BACNET_DEFINES ?= $(MY_BACNET_DEFINES)

# un-comment the next line to build in uci integration
#BACNET_DEFINES += -DBAC_UCI
#UCI_LIB_DIR ?= /usr/local/lib

#BACDL_DEFINE=-DBACDL_ETHERNET=1
#BACDL_DEFINE=-DBACDL_ARCNET=1
#BACDL_DEFINE=-DBACDL_MSTP=1
BACDL_DEFINE?=-DBACDL_BIP=1

# Declare your level of BBMD support
BBMD_DEFINE ?=-DBBMD_ENABLED=1
#BBMD_DEFINE ?= -DBBMD_ENABLED=0
#BBMD_DEFINE ?= -DBBMD_CLIENT_ENABLED

# Passing parameters via command line
MAKE_DEFINE ?=

# Define WEAK_FUNC for [...somebody help here; I can't find any uses of it]
DEFINES = $(BACNET_DEFINES) $(BACDL_DEFINE) $(BBMD_DEFINE) -DWEAK_FUNC=
DEFINES += $(MAKE_DEFINE)

# BACnet Ports Directory
BACNET_PORT ?= linux

# Default compiler settings
OPTIMIZATION = -Os
## EKH: Forcing debugging for now
DEBUGGING = -g3

# EKH: 2018.08.21 - Forcing debug build to avoid 'optimized out' issue with bip-init.c
BUILD=debug
 
# C++ does not require missing prototypes WARNINGS = -Wall -Wmissing-prototypes
# when ready to upgrade to c++ 6: https://gist.github.com/application2000/73fd6f4bf1be6600a2cf9f56315a2d91 
# WARNINGS = -Wall -Wmissing-prototypes -Wduplicated-cond -Wduplicated-branches -Wlogical-op -Wnull-dereference -Wold-style-cast -Wuseless-cast -Wjump-misses-init -Wdouble-promotion -Wshadow -Wformat=2
WARNINGS = -Wall -Wextra -Wno-switch

STANDARDS = -std=gnu11

ifeq (${BUILD},debug)
OPTIMIZATION = -O0
DEBUGGING = -g -DDEBUG_ENABLED=1
ifeq (${BACDL_DEFINE},-DBACDL_BIP=1)
DEFINES += -DBIP_DEBUG
endif
endif

CFLAGS  = $(WARNINGS) $(DEBUGGING) $(OPTIMIZATION) $(STANDARDS) $(INCLUDES) $(DEFINES)

# Export the variables defined here to all subprocesses
# (see http://www.gnu.org/software/automake/manual/make/Special-Targets.html)
.EXPORT_ALL_VARIABLES:

# compiling router-ipv6 a) fails on a prototype conflict b) leaves library corrupted with router build so next make fails
# removing from this build script until we are ready to tackle IPv6
# (this only happens (a) does, when compiling C++, so not noticed with steve's 'standard' C only build process

all: library demos ${DEMO_LINUX}
.PHONY : all library demos clean

library:
	echo calling make lib > $(DEBUGMAKE)
	$(MAKE) -s -C lib all

demos:
	$(MAKE) -C demo all

server:
	$(MAKE) -j -B -C demo server

mstpcap:
	$(MAKE) -B -C demo mstpcap

mstpcrc: library
	$(MAKE) -B -C demo mstpcrc

iam:
	$(MAKE) -B -C demo iam

uevent:
	$(MAKE) -B -C demo uevent

writepropm:
	$(MAKE) -s -B -C demo writepropm

abort:
	$(MAKE) -B -C demo abort

error:
	$(MAKE) -B -C demo error

router-ipv6:
	$(MAKE) -B -s -C demo router-ipv6

# Add "ports" to the build, if desired
ports:	atmega168 bdk-atxx4-mstp at91sam7s stm32f10x
	@echo "Built the ARM7 and AVR ports"

atmega168: ports/atmega168/Makefile
	$(MAKE) -s -C ports/atmega168 clean all

at91sam7s: ports/at91sam7s/Makefile
	$(MAKE) -s -C ports/at91sam7s clean all

stm32f10x: ports/stm32f10x/Makefile
	$(MAKE) -s -C ports/stm32f10x clean all

mstpsnap: ports/linux/mstpsnap.mak
	$(MAKE) -s -C ports/linux -f mstpsnap.mak clean all

bdk-atxx4-mstp: ports/bdk-atxx4-mstp/Makefile
	$(MAKE) -s -C ports/bdk-atxx4-mstp clean all

clean:
	$(MAKE) -s -C lib clean
	$(MAKE) -s -C app clean
	$(MAKE) -s -C demo clean

