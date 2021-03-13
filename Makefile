OBJ_DIR	= ./output
SRC = ./src
TARGET = chip_8.out
CFLAGS = -std=gnu17 -Wall -Werror -Wextra -Wpedantic -Wno-shadow -g -O0
CC = gcc

${OBJ_DIR}/%.o: ${SRC}/%.c
	${CC} -c ${CFLAGS} -o $@ $<

all: ${TARGET}

clean:
	rm ${TARGET}
	rm ${OBJ_DIR}/%.o

OBJLIST = ${OBJ_DIR}/main.o \
          ${OBJ_DIR}/op_codes.o

${TARGET}: ${OBJLIST}
	${CC} -o ${TARGET} ${OBJLIST}

${OBJ_DIR}/main.o: ${SRC}/main.c ${SRC}/op_codes.h
${OBJLIST}/op_codes.o: ${SRC}/op_codes.c ${SRC}/op_codes.h
