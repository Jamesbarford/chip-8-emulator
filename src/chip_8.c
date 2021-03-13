#include "chip_8.h"
#include "op_codes.h"
#include <stdio.h>

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

void init(chip_8_t *chip_8)
{
	chip_8->pc     = MEM_START_ADDR;
	chip_8->opcode = 0;
	chip_8->I      = 0;
	chip_8->sp     = 0;
	
	memset(chip_8->video, 0, sizeof(chip_8->video));
	memset(chip_8->memory, 0, sizeof(chip_8->memory));
	memset(chip_8->registers, 0, sizeof(chip_8->registers));
	memset(chip_8->stack, 0, sizeof(chip_8->stack));

	srand(time(NULL));

	// load fonts 
	for (uint32_t i = 0; i < FONTSET_SIZE; ++i)
		chip_8->memory[FONT_START_ADDR + i] = fontset[i];

	chip_8->delay_timer = 0;
}

void load_rom(char *filename, chip_8_t *chip_8)
{
	struct stat file_stat;
	int fd;
	char *rom;

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

	if ((rom = mmap(NULL, file_stat.st_size, PROT_READ, MAP_PRIVATE, fd, 0)) == MAP_FAILED) 
	{
		fprintf(stderr, "Failed to mmap rom: %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}



	for (int64_t i = 0; i < file_stat.st_size; ++i)
		chip_8->memory[MEM_START_ADDR + i] = rom[i];

	(void)munmap(rom, file_stat.st_size);
	(void)close(fd);
}

void emulate_cycle(chip_8_t *chip_8)
{
	chip_8->opcode = (chip_8->memory[chip_8->pc] << 8) | chip_8->memory[chip_8->pc + 1];

	chip_8->pc += 2;

	printf("opcode: 0x%04X\n", chip_8->opcode);

	// get correct function pointer;	
	call_instruction(chip_8);

	if (chip_8->delay_timer > 0)
		--chip_8->delay_timer;

	// we don't have sound but if we did it would go here
	// if (sound_timer > 0)
	//		--sound_timer;
}

chip_8_t *boot_chip8(char *rom_name)
{
	chip_8_t *c;

	if ((c = (chip_8_t *)malloc(sizeof(chip_8_t))) == NULL)
	{
		fprintf(stderr, "Failed to allocate memory for chip_8_t: %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}

	init(c);
	load_rom(rom_name, c);

	return c;
}

void free_chip_8(chip_8_t *c)
{
	if (c) free(c);
}
