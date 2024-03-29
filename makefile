ver = relase

CFLAGS += -std=c++11

ifeq ($(ver), debug)
CFLAGS += -Ddebug -g
else
CFLAGS += -march=native -O3
endif

ifeq ($(shell uname -s), Darwin)
CC = clang++
else
CC = g++
CFLAGS += -pthread -g
endif

SRC = src
BIN = bin
OBJ = obj
OBJS = ${OBJ}/TrainTransE.o ${OBJ}/main.o
EXE = ${BIN}/main.out


all: ${OBJS}
	${CC} ${CFLAGS} $^ -o ${EXE}

${OBJ}/%.o: ${SRC}/%.cpp
	${CC} ${CFLAGS} -c $< -o $@

clean:
	rm -f ${OBJ}/*.o
	rm -f ${BIN}/*.out
