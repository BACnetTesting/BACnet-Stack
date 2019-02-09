# tools - only if you need them.
# Most platforms have this already defined
# CC = gcc
# AR = ar
# MAKE = make
# SIZE = size
#
# Assumes rm and cp are available

# Directories
BACNET_PORT ?= linux
BACNET_PORT_DIR = ../../ports/${BACNET_PORT}
BACNET_INCLUDE = ../../include
BACNET_INCLUDES3 = ../../bits/util
BACNET_OBJECT = ../../demo/object
BACNET_HANDLER = ../../demo/handler

# BACnet Library
BACNET_LIB_DIR = ../../lib
BACNET_LIB_NAME = bacnet
BACNET_LIB_TARGET = $(BACNET_LIB_DIR)/lib$(BACNET_LIB_NAME).a

# Compiler Setup
INCLUDE1 = -I$(BACNET_PORT_DIR) -I$(BACNET_OBJECT) -I$(BACNET_HANDLER)
INCLUDE2 = -I$(BACNET_INCLUDE)
INCLUDE3 = -I$(BACNET_INCLUDES3)
INCLUDES = $(INCLUDE1) $(INCLUDE2) $(INCLUDE3) \
	-I../../bits/bitsRouter \
	-I../../bits/logging \
	-I../../bits/persist \
	-I../../bits/osLayer/linux \
	-I../../bits/ipc/udp
BACNET_LIB := -L$(BACNET_LIB_DIR),-l$(BACNET_LIB_NAME)

# OS specific builds
ifeq (${BACNET_PORT},linux)
PFLAGS = -pthread
TARGET_EXT =
SYSTEM_LIB=-lc,-lgcc,-lrt,-lm
endif
#build for release (default) or debug
## EKH forcing debug build
DEBUGGING = -g3 
OPTIMIZATION = -Os
ifeq (${BUILD},debug)
OPTIMIZATION = -O0
DEBUGGING = -g -DDEBUG_ENABLED=1
endif

# put all the flags together
CFLAGS := $(STANDARDS) -Wall $(DEBUGGING) $(OPTIMIZATION) $(INCLUDES) $(DEFINES)
LFLAGS := -Wl,$(BACNET_LIB),$(SYSTEM_LIB)

.EXPORT_ALL_VARIABLES:

SUBDIRS = projectName

.PHONY : all projectName

TARGETS = all projectName clean

$(TARGETS): %: $(patsubst %, %.%, $(SUBDIRS))

$(foreach TGT, $(TARGETS), $(patsubst %, %.$(TGT), $(SUBDIRS))):
	$(MAKE) -s -b -C $(subst ., , $@)

