ver = debug

ifeq ($(ver), debug)
CFLAGS += -Ddebug -g
else
CFLAGS += -O3 march = native
endif

ifeq ($(shell uname -s), Darwin)
CC = clang++
else
CC = g++
CFLAGS += -pthread
endif

SRC = src
BIN = bin
OBJ = obj
OBJS = ${OBJ}/word2vec.o ${OBJ}/transE.o
EXE = ${BIN}/main.out


all: ${OBJS}
	${CC} ${CFLAGS} $^ -o ${EXE}

${OBJ}/%.o: ${SRC}/%.c
	${CC} ${CFLAGS} -c $< -o $@

clean:
	rm -f ${OBJ}/*.o
	rm -f ${BIN}/*.out
