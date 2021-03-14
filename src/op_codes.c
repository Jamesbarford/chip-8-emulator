#include "op_codes.h"
#include "chip_8.h"

#define VX(x) ((x >> 8) & 0xF)
#define VY(y) ((y >> 4) & 0xF)

/** OP START
 *
 * nnn or addr -> 12-bit value, the lowest 12 bits of the instruction
 * n or nibble -> 4-bit  value, the lowest 4 bits of the instruction
 * x           -> 4-bit  value, the lower 4 bits of the high byte of the instruction
 * y           -> 4-bit  value, the upper 4 bits of the low byte of the instruction
 * kk or byte  -> 8-bit  value, the lowest 8 bits of the instruction
 *
 * instruction set source: http://devernay.free.fr/hacks/chip8/C8TECH10.HTM
 * **/

static void OP_00E0(chip_8_t *chip_8)
{// CLS
	memset(chip_8->video, 0, sizeof(chip_8->video));
}

static void OP_00EE(chip_8_t *chip_8)
{// RET
	--chip_8->sp;
	chip_8->pc = chip_8->stack[chip_8->sp];
}

static void OP_1nnn(chip_8_t *chip_8)
{// JP addr
	chip_8->pc = chip_8->opcode & 0xFFF;
}

static void OP_2nnn(chip_8_t *chip_8)
{// CALL addr
	chip_8->stack[chip_8->sp] = chip_8->pc;
	++chip_8->sp;
	chip_8->pc = chip_8->opcode & 0xFFF;
}

static void OP_3xkk(chip_8_t *chip_8)
{// SNE -> skip next instruction if equal
	uint8_t kk = chip_8->opcode & 0xFF;

	if (chip_8->registers[VX(chip_8->opcode)] == kk)
		chip_8->pc += 2;
}

static void OP_4xkk(chip_8_t *chip_8)
{// SNE -> skip next instruction if Not equal
	uint8_t kk = chip_8->opcode & 0xFF;

	if (chip_8->registers[VX(chip_8->opcode)] != kk)
		chip_8->pc += 2;
}

static void OP_5xy0(chip_8_t *chip_8)
{// SE -> skip next instruction if equal
	if (chip_8->registers[VX(chip_8->opcode)] == chip_8->registers[VY(chip_8->opcode)])
		chip_8->pc += 2;
}

static void OP_6xkk(chip_8_t *chip_8)
{// LD (bit like MOV) Vx, byte
	uint8_t kk = chip_8->opcode & 0xFF;

	chip_8->registers[VX(chip_8->opcode)] = kk;
}

static void OP_7xkk(chip_8_t *chip_8)
{// ADD 
	uint8_t kk = chip_8->opcode & 0xFF;

	chip_8->registers[VX(chip_8->opcode)] += kk;
}

static void OP_8xy0(chip_8_t *chip_8)
{// SET Vx = Vy
	chip_8->registers[VX(chip_8->opcode)] = chip_8->registers[VY(chip_8->opcode)]; 
}

static void OP_8xy1(chip_8_t *chip_8)
{// Vx = Vx OR Vy
	chip_8->registers[VX(chip_8->opcode)] |= chip_8->registers[VY(chip_8->opcode)]; 
}

static void OP_8xy2(chip_8_t *chip_8)
{// Vx = Vx AND Vy
	chip_8->registers[VX(chip_8->opcode)] &= chip_8->registers[VY(chip_8->opcode)]; 
}

static void OP_8xy3(chip_8_t *chip_8)
{// Vx = Vx XOR Vy
	chip_8->registers[VX(chip_8->opcode)] ^= chip_8->registers[VY(chip_8->opcode)]; 
}

static void OP_8xy4(chip_8_t *chip_8)
{// ADD Vx Vy,  set VF = carry flag, only lower 8 bits stored in Vx
	uint16_t result = chip_8->registers[VX(chip_8->opcode)] + chip_8->registers[VY(chip_8->opcode)];
	
	chip_8->registers[0xF] = result > 0xFF ? 1 : 0;
	chip_8->registers[VX(chip_8->opcode)] = result & 0xFF;
}

static void OP_8xy5(chip_8_t *chip_8)
{// SUB Vx,Vy,  Vx = Vx - Vy, set VF = NOT borrow
	uint8_t Vx = VX(chip_8->opcode);
	uint8_t Vy = VY(chip_8->opcode);

	chip_8->registers[0xF] = chip_8->registers[Vx] > chip_8->registers[Vy] ? 1 : 0;
	chip_8->registers[Vx] -= chip_8->registers[Vy];
}

static void OP_8xy6(chip_8_t *chip_8)
{// Vx = Vx SHR1 if the least significant bit of Vx is 1. Then Vx is divided by 2	
	uint8_t Vx = VX(chip_8->opcode);
	
	chip_8->registers[0xF] = (chip_8->registers[Vx] & 0x1);
	chip_8->registers[Vx] /= 2;
}

static void OP_8xy7(chip_8_t *chip_8)
{// Vx = Vy - Vx, VF = NOT BORROW
	uint8_t Vx = VX(chip_8->opcode);
	uint8_t Vy = VY(chip_8->opcode);

	chip_8->registers[0xF] = chip_8->registers[Vy] > chip_8->registers[Vx] ? 1 : 0;
	chip_8->registers[Vx] = chip_8->registers[Vy] - chip_8->registers[Vx];
}

static void OP_8xyE(chip_8_t *chip_8)
{// Vx = Vx SHL 1. if most significant bit is 1, VF is set to one. Then Vx * 2
	uint8_t Vx = VX(chip_8->opcode);
	
	chip_8->registers[0xF] = (chip_8->registers[Vx] & 0x80) >> 7;
	chip_8->registers[Vx] *= 2;
}

static void OP_9xy0(chip_8_t *chip_8)
{// SNE Vx, Vy. skip if Vx != Vy
	if (chip_8->registers[VX(chip_8->opcode)] != chip_8->registers[VY(chip_8->opcode)])
		chip_8->pc += 2;
}

static void OP_Annn(chip_8_t *chip_8)
{// SET I = nnn. Register I set to nnn
	chip_8->I = chip_8->opcode & 0xFFF;
}

static void OP_Bnnn(chip_8_t *chip_8)
{// JP nnn + V0
	chip_8->pc = chip_8->registers[0] + (chip_8->opcode & 0xFFF);
}

static void OP_Cxkk(chip_8_t *chip_8)
{// SET Vx = Random byte AND kk.
	uint16_t kk = chip_8->opcode & 0x00FF;

	chip_8->registers[VX(chip_8->opcode)] = (rand() % 255) & kk;
}

static void OP_Dxyn(chip_8_t *chip_8)
{// Display n-byte sprite starting at memory Location I at (Vx, Vy), set VF = collision
	uint8_t height = chip_8->opcode & 0xF;
	uint8_t x = chip_8->registers[VX(chip_8->opcode)] % V_WIDTH;
	uint8_t y = chip_8->registers[VY(chip_8->opcode)] % V_HEIGHT;

	chip_8->registers[0xF] = 0;

	for (uint32_t row = 0; row < height; ++row)
	{
		uint8_t sprite_byte = chip_8->memory[chip_8->I + row];

		for (uint32_t col = 0; col < 8; ++col)
		{
			uint8_t sprite_pixel = sprite_byte & (0x80 >> col);
			uint32_t *pixel = &chip_8->video[(y + row) * V_WIDTH + (x + col)];

			if (sprite_pixel)
			{
				if (*pixel == 0xFFFFFFFF)
					chip_8->registers[0xF] = 1;

				*pixel ^= 0xFFFFFFFF;
			}
		}
	}
}

static void OP_Ex9E(chip_8_t *chip_8)
{// SKP Vx, if key pressed
	uint8_t key = chip_8->registers[VX(chip_8->opcode)];
	if (chip_8->keypad[key])
		chip_8->pc += 2;
}

static void OP_ExA1(chip_8_t *chip_8)
{// skip if key not pressed
	uint8_t key = chip_8->registers[VX(chip_8->opcode)];
	if (!chip_8->keypad[key])
		chip_8->pc += 2;
}

static void OP_Fx07(chip_8_t *chip_8)
{
	chip_8->registers[VX(chip_8->opcode)] = chip_8->delay_timer;
}

static void OP_Fx0A(chip_8_t *chip_8)
{
	uint8_t Vx = VX(chip_8->opcode);

	if      (chip_8->keypad[0])  chip_8->registers[Vx] = 0;
	else if (chip_8->keypad[1])  chip_8->registers[Vx] = 1;
	else if (chip_8->keypad[2])  chip_8->registers[Vx] = 2;
	else if (chip_8->keypad[3])  chip_8->registers[Vx] = 3;
	else if (chip_8->keypad[4])  chip_8->registers[Vx] = 4;
	else if (chip_8->keypad[5])  chip_8->registers[Vx] = 5;
	else if (chip_8->keypad[6])  chip_8->registers[Vx] = 6;
	else if (chip_8->keypad[7])  chip_8->registers[Vx] = 7;
	else if (chip_8->keypad[8])  chip_8->registers[Vx] = 8;
	else if (chip_8->keypad[9])  chip_8->registers[Vx] = 9;
	else if (chip_8->keypad[10]) chip_8->registers[Vx] = 10;
	else if (chip_8->keypad[11]) chip_8->registers[Vx] = 11;
	else if (chip_8->keypad[12]) chip_8->registers[Vx] = 12;
	else if (chip_8->keypad[13]) chip_8->registers[Vx] = 13;
	else if (chip_8->keypad[14]) chip_8->registers[Vx] = 14;
	else if (chip_8->keypad[15]) chip_8->registers[Vx] = 15;
	else
		chip_8->pc -= 2;
}

static void OP_Fx15(chip_8_t *chip_8)
{// delay_timer = vx
	chip_8->delay_timer = chip_8->registers[VX(chip_8->opcode)];
}

static void OP_Fx18()
{// set the sound timer (we don't have sound)
	return;
}

static void OP_Fx1E(chip_8_t *chip_8)
{// set I to I + Vx
	chip_8->I += chip_8->registers[VX(chip_8->opcode)];
}

static void OP_Fx29(chip_8_t *chip_8)
{// LD F, Vx -> set I to be the location of the sprite for digit Vx
	uint8_t digit = chip_8->registers[VX(chip_8->opcode)];

	chip_8->I = FONT_START_ADDR + (5 * digit);
}

static void OP_Fx33(chip_8_t *chip_8)
{// LD B, Vx -> store BCD representation of Vx in memory locations, I, I + 1 and I + 2 
	uint8_t value = chip_8->registers[VX(chip_8->opcode)];

	chip_8->memory[chip_8->I + 2] = value % 10;
	value /= 10;

	chip_8->memory[chip_8->I + 1] = value % 10;
	value /= 10;

	chip_8->memory[chip_8->I] = value % 10;
}

static void OP_Fx55(chip_8_t *chip_8)
{
	for (uint8_t i = 0; i <= VX(chip_8->opcode); ++i)
		chip_8->memory[chip_8->I + i] = chip_8->registers[i];
}

static void OP_Fx65(chip_8_t *chip_8)
{
	for (uint8_t i = 0; i <= VX(chip_8->opcode); ++i)
		chip_8->registers[i] = chip_8->memory[chip_8->I + i];
}

// wrapper for switch case deciding which op to call predicated on the opcode
void call_instruction(chip_8_t *chip_8)
{// create virtual jumptable
	switch (chip_8->opcode & 0xF000) {
		case 0x0000: {
			switch (chip_8->opcode & 0x000F) {
				case 0x0000: OP_00E0(chip_8); return;
				case 0x000E: OP_00EE(chip_8); return;
				default: goto invalid; return;
			}
			return;
		}
		case 0x1000: OP_1nnn(chip_8); return;
		case 0x2000: OP_2nnn(chip_8); return;
		case 0x3000: OP_3xkk(chip_8); return;
		case 0x4000: OP_4xkk(chip_8); return;
		case 0x5000: OP_5xy0(chip_8); return;
		case 0x6000: OP_6xkk(chip_8); return;
		case 0x7000: OP_7xkk(chip_8); return;
		case 0x8000: {
			switch (chip_8->opcode & 0x000F) {
				case 0x0000: OP_8xy0(chip_8); return;
				case 0x0001: OP_8xy1(chip_8); return;
				case 0x0002: OP_8xy2(chip_8); return;
				case 0x0003: OP_8xy3(chip_8); return;
				case 0x0004: OP_8xy4(chip_8); return;
				case 0x0005: OP_8xy5(chip_8); return;
				case 0x0006: OP_8xy6(chip_8); return;
				case 0x0007: OP_8xy7(chip_8); return;
				case 0x000E: OP_8xyE(chip_8); return;
				default: goto invalid; return;
			}
			return;
		}
		case 0x9000: OP_9xy0(chip_8); return;
		case 0xA000: OP_Annn(chip_8); return;
		case 0xB000: OP_Bnnn(chip_8); return;
		case 0xC000: OP_Cxkk(chip_8); return;
		case 0xD000: OP_Dxyn(chip_8); return;
		case 0xE000: {
			switch (chip_8->opcode & 0x000F) {
				case 0x000E: OP_Ex9E(chip_8); return;
				case 0x0001: OP_ExA1(chip_8); return;
				default: goto invalid; return;
			}
			return;
		}
		case 0xF000: {
			switch (chip_8->opcode & 0x00FF) {
				case 0x0007: OP_Fx07(chip_8); return;
				case 0x000A: OP_Fx0A(chip_8); return;
				case 0x0015: OP_Fx15(chip_8); return;
				case 0x0018: OP_Fx18(); return;
				case 0x001E: OP_Fx1E(chip_8); return;
				case 0x0029: OP_Fx29(chip_8); return;
				case 0x0033: OP_Fx33(chip_8); return;
				case 0x0055: OP_Fx55(chip_8); return;
				case 0x0065: OP_Fx65(chip_8); return;
				default: goto invalid; return;
			}
			return;
		}
		default: goto invalid; return;

	}

invalid:
	printf("Unknown opcode: 0x%04X\n", chip_8->opcode); 
}
