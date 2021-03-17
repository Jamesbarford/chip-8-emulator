OBJ_DIR	= ./output
SRC = ./src
TARGET = chip_8.out
CFLAGS = -std=c11 -Wall -Werror -Wextra -Wpedantic -Wno-shadow -O2
CC = cc

${OBJ_DIR}/%.o: ${SRC}/%.c
	${CC} -c ${CFLAGS} -o $@ $<

all: ${TARGET}

clean:
	rm ${OBJ_DIR}/*.o
	rm ${TARGET}

OBJLIST = ${OBJ_DIR}/main.o \
          ${OBJ_DIR}/chip_8.o \
          ${OBJ_DIR}/op_codes.o \
          ${OBJ_DIR}/display.o

${TARGET}: ${OBJLIST}
	${CC} -o ${TARGET} ${OBJLIST} -lSDL2main -lSDL2

${OBJ_DIR}/main.o: ${SRC}/main.c ${SRC}/chip_8.h ${SRC}/display.h
${OBJLIST}/chip_8.o: ${SRC}/chip_8.c ${SRC}/chip_8.h
${OBJLIST}/op_codes.o: ${SRC}/op_codes.c ${SRC}/chip_8.h ${SRC}/op_codes.h
${OBJLIST}/display.o: ${SRC}/display.c ${SRC}/display.h
