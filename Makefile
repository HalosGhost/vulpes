CFLAGS  +=  -Os -Wall
PROG    =   vulpes
PREFIX  ?=  /usr
VER     =   1.0
LIBS	  =	-lcurl

${PROG}: ${PROG}.c
	@${CC} ${CFLAGS} ${LIBS} -o ${PROG} ${PROG}.c
	@strip ${PROG}

clean:
	@rm -f ${PROG}

install: ${PROG}
	@install -Dm755 ${PROG} ${DESTDIR}${PREFIX}/bin/${PROG}
