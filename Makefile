TARGET=yjson
CPPFLAGS=-Wall -std=c99
LIBS=-lm
${TARGET}:yjson.c
	gcc ${CPPFLAGS} $< -o $@ ${LIBS}
run:
	./${TARGET}
clean:
	rm -f ./${TARGET} *.o
