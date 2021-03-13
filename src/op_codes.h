#ifndef OP_CODES_H
#define OP_CODES_H

#include <stdio.h>
#include <stdint.h>
#include <errno.h>
#include <string.h>

#include "chip_8.h"

void init(chip_8_t *chip_8);
void load_rom(char *filename, chip_8_t *chip_8);
void emulate_cycle(chip_8_t *chip_8);
void call_instruction(chip_8_t *chip_8);

#endif
