#ifndef CHIP8_H
#define CHIP8_H

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <string.h>
#include <time.h>

#define MEM_START_ADDR  0x200
#define FONT_START_ADDR 0x50

#define MEMORY_SIZE  4096
#define FONTSET_SIZE 80

#define V_WIDTH  64
#define V_HEIGHT 32

#define MEMORY_SIZE 4096

/** 
 * General Registers from 0x0 to 0xF
 * The 0xF register is not to be used by a program as thats a flag:
 * - the carry flag, indicates that the result of an operation cannot fit into the size of register
 * - the borrow flag, 
 *
 * 0x000 to 0x200 is reserved memory
 **/

typedef struct chip_8_t {
	uint8_t registers[16];
	uint8_t memory[MEMORY_SIZE];
	uint8_t sp;         /* stack pointer */
	uint8_t delay_timer;
	uint8_t keypad[16];
	uint16_t I; /* generally used for memory addresses  */
	uint16_t pc; /* program counter  */
	uint16_t stack[16];
	uint32_t video[V_WIDTH * V_HEIGHT];
	uint16_t opcode;
} chip_8_t;

void boot_chip8(chip_8_t *chip_8, char *rom_name);
void emulate_cycle(chip_8_t *chip_8);
void print_video(chip_8_t *chip_8);

#endif
