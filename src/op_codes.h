#ifndef OP_CODES_H
#define OP_CODES_H

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <string.h>
#include <time.h>

void init();
void load_rom(char *filename);
void emulate_cycle();

#endif
