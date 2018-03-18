TARGET = testPub

SRCS = testPub.c mqttIPC.c

OBJS = ${SRCS:.c=.o}

all: test

${TARGET}: ${OBJS} Makefile
	${CC} ${PFLAGS} ${OBJS} ${LFLAGS} -o $@

.c.o:
	${CC} -c ${CFLAGS} $*.c -o $@

clean:
	rm -f *.o
	rm -f testPub
	

