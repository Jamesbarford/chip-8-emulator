#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <time.h>

/**
 * General Registers from 0x0 to 0xF
 * The 0xF register is not to be used by a program as thats a flag:
 * - the carry flag, indicates that the result of an operation cannot fit into the size of register
 * - the borrow flag, 
 *
 * 0x000 to 0x200 is reserved memory
 **/
#define MEM_START_ADDR  0x200
#define FONT_START_ADDR 0x50

#define MEMORY_SIZE  4096
#define FONTSET_SIZE 80

#define V_WIDTH  400
#define V_HEIGHT 400

#define VX(x) ((x >> 12) & 0xF)
#define VY(y) ((y >> 4) & 0xF)

uint8_t registers[16];
uint8_t memory[MEMORY_SIZE];
uint8_t sp;         /* stack pointer */
uint8_t delay_timer;
uint8_t keypad[16];
uint16_t I; /* generally used for memory addresses  */
uint16_t pc; /* program counter  */
uint16_t stack[16];
uint16_t opcode;
uint32_t video[64 * 32];

uint8_t fontset[FONTSET_SIZE] = 
{
	0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
	0x20, 0x60, 0x20, 0x20, 0x70, // 1
	0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
	0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
	0x90, 0x90, 0xF0, 0x10, 0x10, // 4
	0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
	0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
	0xF0, 0x90, 0xF0, 0x10, 0x40, // 7
	0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
	0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
	0xF0, 0x90, 0xF0, 0x90, 0x90, // A
	0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
	0xF0, 0x80, 0x80, 0x80, 0xF0, // C
	0xE0, 0x90, 0x90, 0x90, 0xE0, // D
	0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
	0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};

uint8_t rand_byte()
{
	return rand() % 255;
}

void init()
{
	pc     = MEM_START_ADDR;
	opcode = 0;
	I      = 0;
	sp     = 0;
	
	memset(video, 0, sizeof(video));
	memset(memory, 0, sizeof(memory));
	memset(registers, 0, sizeof(registers));
	memset(stack, 0, sizeof(stack));

  // load fonts 
	for (uint32_t i = 0; i < FONTSET_SIZE; ++i)
		memory[FONT_START_ADDR + i] = fontset[i];

	delay_timer = 0;
}

void load_rom(char *filename)
{
	struct stat file_stat;
	int fd, idx = 0;
	ssize_t bytes;
	char c;

	if ((fd = open(filename, O_RDONLY)) == -1)
	{
		fprintf(stderr, "Failed to open file: %s", strerror(errno));
		exit(EXIT_FAILURE);
	}

	if ((stat(filename, &file_stat) == -1))
	{
		fprintf(stderr, "Failed to get file stats %s", strerror(errno));
		exit(EXIT_FAILURE);
	}

	char buf[file_stat.st_size];

	// this could load it straight into memory and error if the size is too big
	if ((bytes = read(fd, buf, file_stat.st_size)) == -1)
	{
		fprintf(stderr, "Failed to read file '%s': %s", filename, strerror(errno));
		exit(EXIT_FAILURE);
	}

	buf[bytes] = '\0';
	
	while ((c = buf[idx++]) != '\0')
		memory[MEM_START_ADDR + (idx - 1)] = c;

	(void)close(fd);
}

void emulate_cycle()
{
	opcode = memory[pc] << 8 | memory[pc + 1];

	switch (opcode & 0xF000) {
		case 0xA000:
			break;
		default:
			fprintf(stderr, "Invalid opcode: 0x%X\n", opcode);
			exit(EXIT_FAILURE);
	}

	if (delay_timer > 0)
		--delay_timer;

	// sound timer;
}

/** OP CODES START
 *
 * nnn or addr -> 12-bit value, the lowest 12 bits of the instruction
 * n or nibble -> 4-bit  value, the lowest 4 bits of the instruction
 * x           -> 4-bit  value, the lower 4 bits of the high byte of the instruction
 * y           -> 4-bit  value, the upper 4 bits of the low byte of the instruction
 * kk or byte  -> 8-bit  value, the lowest 8 bits of the instruction
 *
 * instruction set source: http://devernay.free.fr/hacks/chip8/C8TECH10.HTM
 * **/

void OP_00E0()
{// CLS
	memset(video, 0, sizeof(video));
}

void OP_00EE()
{// RET
	--sp;
	pc = stack[sp];
}

void OP_lnnn()
{// JP addr
	pc = opcode & 0xFFF;
}

void OP_2nnn()
{// CALL addr
	stack[sp] = pc;
	++sp;
	pc = opcode & 0xFFF;
}

void OP_3xkk()
{// SNE -> skip next instruction if equal
	uint8_t kk = opcode & 0xFF;

	if (registers[VX(opcode)] == kk)
		pc += 2;
}

void OP_4xkk()
{// SNE -> skip next instruction if Not equal
	uint8_t kk = opcode & 0xFF;

	if (registers[VX(opcode)] != kk)
		pc += 2;
}

void OP_5xy0()
{// SE -> skip next instruction if equal
	if (registers[VX(opcode)] == registers[VY(opcode)])
		pc += 2;
}

void OP_6xkk()
{// LD (bit like MOV) Vx, byte
	uint8_t kk = opcode & 0xFF;

	registers[VX(opcode)] = kk;
}

void OP_7xkk()
{// ADD 
	uint8_t kk = opcode & 0xFF;

	registers[VX(opcode)] += kk;
}

void OP_8xy0()
{// SET Vx = Vy
	registers[VX(opcode)] = registers[VY(opcode)]; 
}

void OP_8xy1()
{// Vx = Vx OR Vy
	registers[VX(opcode)] |= registers[VY(opcode)]; 
}

void OP_8xy2()
{// Vx = Vx AND Vy
	registers[VX(opcode)] &= registers[VY(opcode)]; 
}

void OP_8xy3()
{// Vx = Vx XOR Vy
	registers[VX(opcode)] ^= registers[VY(opcode)]; 
}

void OP_8xy4()
{// ADD Vx Vy,  set VF = carry flag, only lower 8 bits stored in Vx
	uint32_t result = registers[VX(opcode)] + registers[VY(opcode)];
	
	registers[0xF] = result > 0xFF ? 1 : 0;
	registers[VX(opcode)] = result & 0xFF;
}

void OP_8xy5()
{// SUB Vx,Vy,  Vx = Vx - Vy, set VF = NOT borrow
	uint8_t Vx = VX(opcode);
	uint8_t Vy = VY(opcode);

	registers[0xF] = registers[Vx] > registers[Vy] ? 1 : 0;
	registers[Vx] -= registers[Vy];
}

void OP_8xy6()
{// Vx = Vx SHR1 if the least significant bit of Vx is 1. Then Vx is divided by 2	
	uint8_t Vx = VX(opcode);
	
	registers[0xF] = registers[Vx] & 0x1;
	registers[Vx] /= 2;
}

void OP_8xy7()
{// Vx = Vy - Vx, VF = NOT BORROW
	uint8_t Vx = VX(opcode);
	uint8_t Vy = VY(opcode);

	registers[0xF] = registers[Vy] > registers[Vx] ? 1 : 0;
	registers[Vx] = registers[Vy] - registers[Vx];
}

void OP_8xyE()
{// Vx = Vx SHL 1. if most significant bit is 1, VF is set to one. Then Vx * 2
	uint8_t Vx = VX(opcode);
	
	registers[0xF] = (registers[Vx] >> (3 << 2) & 0xF) & 0x1;
	registers[Vx] *= 2;
}

void OP_9xy0()
{// SNE Vx, Vy. skip if Vx != Vy
	if (registers[VX(opcode)] != registers[VY(opcode)])
		pc += 2;
}

void OP_Annn()
{// SET I = nnn. Register I set to nnn
	I = opcode & 0xFFF;
}

void OP_Bnnn()
{// JP nnn + V0
	pc = registers[0] + (opcode & 0xFFF);
}

void OP_Cxkk()
{// SET Vx = Random byte AND kk.
	registers[VX(opcode)] = rand_byte() & 0xF;
}

void OP_Dxyn()
{// Display n-byte sprite starting at memory Location I at (Vx, Vy), set VF = collision
	uint8_t height = opcode & 0xF;
	uint8_t sprite_byte, sprite_pixel;
	uint8_t x = registers[VX(opcode)] % V_WIDTH;
	uint8_t y = registers[VY(opcode)] % V_HEIGHT;
	uint32_t *pixel;

	registers[0xF] = 0;

	for (uint32_t row = 0; row < height; ++row)
	{
		sprite_byte = memory[I + row];

		for (uint32_t col = 0; col < 8; ++col)
		{
			sprite_pixel = sprite_byte & (0x80 >> col);
			pixel = &video[(y + row) * V_WIDTH + (x + col)];

			if (sprite_pixel)
			{
				if (*pixel == 0xFFFFFFFF)
					registers[0xF] = 1;

				*pixel ^= 0xFFFFFFFF;
			}
		}
	}
}

void OP_Ex9E()
{// SKP Vx, if key pressed
	uint8_t key = registers[VX(opcode)];
	if (keypad[key])
		pc += 2;
}

void OP_ExA1()
{// skip if key not pressed
	uint8_t key = registers[VX(opcode)];
	if (!keypad[key])
		pc += 2;
}

void OP_Fx07()
{
	registers[VX(opcode)] = delay_timer;
}

void OP_Fx0A()
{
	uint8_t Vx = VX(opcode);

	if (keypad[0])       registers[Vx] = 0;
	else if (keypad[1])  registers[Vx] = 1;
	else if (keypad[2])  registers[Vx] = 2;
	else if (keypad[3])  registers[Vx] = 3;
	else if (keypad[4])  registers[Vx] = 4;
	else if (keypad[5])  registers[Vx] = 5;
	else if (keypad[6])  registers[Vx] = 6;
	else if (keypad[7])  registers[Vx] = 7;
	else if (keypad[8])  registers[Vx] = 8;
	else if (keypad[9])  registers[Vx] = 9;
	else if (keypad[10]) registers[Vx] = 10;
	else if (keypad[11]) registers[Vx] = 11;
	else if (keypad[12]) registers[Vx] = 12;
	else if (keypad[13]) registers[Vx] = 13;
	else if (keypad[14]) registers[Vx] = 14;
	else if (keypad[15]) registers[Vx] = 15;
	else
		pc -= 2;
}

void OP_Fx15()
{// delay_timer = vx
	delay_timer = registers[VX(opcode)];
}

void OP_Fx18()
{// set the sound timer (we don't have sound)
	return;
}

void OP_Fx1E()
{// set I to I + Vx
	I += registers[VX(opcode)];
}

void OP_Fx29()
{// LD F, Vx -> set I to be the location of the sprite for digit Vx
	uint8_t digit = registers[VX(opcode)];

	I = FONT_START_ADDR + (5 * digit);
}

void OP_Fx33()
{// LD B, Vx -> store BCD representation of Vx in memory locations, I, I + 1 and I + 2 
	uint8_t value = registers[VX(opcode)];

	memory[I + 2] = value % 10;
	value /= 10;

	memory[I + 1] = value % 10;
	value /= 10;

	memory[I] = value % 10;
}

void OP_Fx55()
{
	for (uint8_t i = 0; i <= registers[VX(opcode)]; ++i)
		memory[I + i] = registers[i];
}

void OP_Fx65()
{
	for (uint8_t i = 0; i <= registers[VX(opcode)]; ++i)
		registers[i] = memory[I + i];
}


/** OP CODES END **/
int main(void)
{
	srand(time(NULL));
//	load_rom("./file.txt");
	printf("hello world\n");
}
