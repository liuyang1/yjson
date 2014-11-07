TARGET=yjson
CPPFLAGS=-Wall
${TARGET}:yjson.c
	gcc ${CPPFLAGS} $< -o $@
