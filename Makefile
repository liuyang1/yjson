TARGET=yjson
CPPFLAGS=-Wall
LIBS=-lm
${TARGET}:yjson.c
	gcc ${CPPFLAGS} $< -o $@ ${LIBS}
run:
	./${TARGET}
clean:
	rm -f ./${TARGET} *.o
