#Makefile to build BACnet Application using GCC compiler

# tools - only if you need them.
# Most platforms have this already defined
# CC = gcc
# AR = ar
# MAKE = make
# SIZE = size
#
# Assumes rm and cp are available

# Executable file name
TARGET = bacrp

TARGET_BIN = ${TARGET}$(TARGET_EXT)

SRCS = main.c \
	../object/netport.c \
	../object/device-client.c

OBJS = ${SRCS:.c=.o}

all: ${BACNET_LIB_TARGET} Makefile ${TARGET_BIN}
	echo libs=${BACNET_LIB_DIR}> /dev/pts/0

${TARGET_BIN}: ${OBJS} Makefile ${BACNET_LIB_TARGET}
	echo libs=${BACNET_LIB_TARGET} > /dev/pts/0
	${CC} ${PFLAGS} ${OBJS} ${BACNET_LIB_TARGET} ${LFLAGS} -o $@
	size $@
	cp $@ ../../bin

.PHONY:lib
lib: ${BACNET_LIB_TARGET}

${BACNET_LIB_TARGET}:
	echo libs=${BACNET_LIB_DIR}> /dev/pts/0
	( cd ${BACNET_LIB_DIR} ; $(MAKE) clean ; $(MAKE) )

.c.o:
	echo cflags= ${CFLAGS} > /dev/pts/0
	${CC} -c ${CFLAGS} $*.c -o $@

depend:
	rm -f .depend
	${CC} -MM ${CFLAGS} *.c >> .depend

clean:
	rm -rf core ${TARGET_BIN} ${OBJS} ${BACNET_LIB_TARGET}

include: .depend
