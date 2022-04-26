DEFINES = -Deddie
# INCLUDES ?=

DEBUGGING = -g3 
OPTIMIZATION = -Os
ifeq (${BUILD},debug)
OPTIMIZATION = -O0
DEBUGGING = -g -DDEBUG_ENABLED=1
endif

# put all the flags together
CFLAGS := $(STANDARDS) -Wall $(DEBUGGING) $(OPTIMIZATION) $(INCLUDES) $(DEFINES)
LFLAGS := -Wl,/usr/local/lib/libmosquitto.so.1

.EXPORT_ALL_VARIABLES:

.PHONY : mqtt

all: mqtt   

# -C Step into <directory>
# -b compatibility - ignore
# -s Silent

mqtt:
	$(MAKE) -s -b -C mqtt

clean:
	$(MAKE) -s -b -C mqtt clean

