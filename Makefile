TARGET=yjson
CPPFLAGS=-std=c99 -Wall -Werror
LIBS=-lm
${TARGET}:yjson.o test_yjson.o
	gcc ${CPPFLAGS} $^ -o $@ ${LIBS}
yjson.o: yjson.c yjson.h
	gcc ${CPPFLAGS} -c yjson.c -o $@
test_yjson.o: test_yjson.c yjson.h
	gcc ${CPPFLAGS} -c test_yjson.c -o $@
run:
	./${TARGET}
clean:
	rm -f ./${TARGET} *.o
