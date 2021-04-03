#ifndef DISPLAY_H
#define DISPLAY_H

#include <SDL2/SDL.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>

#define True 1
#define False 0

typedef unsigned short BOOL;

typedef struct display_t {
	SDL_Window *window;
	SDL_Renderer *renderer;
	SDL_Texture *texture;
} display_t;

void alloc_display(display_t *display, char *win_name, uint32_t win_width, uint32_t win_height, uint32_t texture_width, uint32_t texture_height);
void free_display(display_t *display);
void update_display(display_t *display, uint32_t *video, int pitch);
BOOL handle_input(uint8_t *keys);

#endif
