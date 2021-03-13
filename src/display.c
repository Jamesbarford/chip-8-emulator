#include "display.h"

display_t *alloc_display(char *win_name, uint32_t win_width, uint32_t win_height, uint32_t texture_width, uint32_t texture_height)
{
	display_t *display;

	if (( display = (display_t *)malloc(sizeof(display_t))) == NULL)
	{
		fprintf(stderr, "Failed to allocate memory for display: %s\n", strerror(errno));
		exit(EXIT_FAILURE);	
	}

	SDL_Init(SDL_INIT_VIDEO);
	
	display->window = SDL_CreateWindow(win_name, 0, 0, win_width, win_height, SDL_WINDOW_SHOWN);
	display->renderer = SDL_CreateRenderer(display->window, -1, SDL_RENDERER_ACCELERATED);
	display->texture = SDL_CreateTexture(display->renderer, SDL_PIXELFORMAT_RGB888, SDL_TEXTUREACCESS_STREAMING, texture_width, texture_height);

	return display;
}

void update_display(display_t *display, uint32_t *video, int pitch)
{
	SDL_UpdateTexture(display->texture, NULL, video, pitch);
	SDL_RenderClear(display->renderer);
	SDL_RenderCopy(display->renderer, display->texture, NULL, NULL);
	SDL_RenderPresent(display->renderer);
}

void free_display(display_t *display)
{
	SDL_DestroyRenderer(display->renderer);
	SDL_DestroyTexture(display->texture);
	SDL_DestroyWindow(display->window);
	if (display)
		free(display);
	SDL_Quit();
}

BOOL handle_input(uint8_t *keys)
{
	BOOL terminate = False;
	SDL_Event evt;

	while (SDL_PollEvent(&evt))
	{
		switch (evt.type)
		{
			case SDL_QUIT: terminate = True; break;
			case SDL_KEYDOWN:
			{
				switch (evt.key.keysym.sym)
				{
					case SDLK_ESCAPE: terminate = True; break;
					case SDLK_x: keys[0] = 1; break;
					case SDLK_1: keys[1] = 1; break;
					case SDLK_2: keys[2] = 1; break;
					case SDLK_3: keys[3] = 1; break;
					case SDLK_q: keys[4] = 1; break;
					case SDLK_w: keys[5] = 1; break;
					case SDLK_e: keys[6] = 1; break;
					case SDLK_a: keys[7] = 1; break;
					case SDLK_s: keys[8] = 1; break;
					case SDLK_d: keys[9] = 1; break;
					case SDLK_z: keys[10] = 1; break;
					case SDLK_c: keys[11] = 1; break;
					case SDLK_4: keys[12] = 1; break;
					case SDLK_r: keys[13] = 1; break;
					case SDLK_f: keys[14] = 1; break;
					case SDLK_v: keys[15] = 1; break;
					default: break;
				}
				break;
			}
			case SDL_KEYUP:
			{
				switch (evt.key.keysym.sym)
				{
					case SDLK_ESCAPE: terminate = True; break;
					case SDLK_x: keys[0] = 0; break;
					case SDLK_1: keys[1] = 0; break;
					case SDLK_2: keys[2] = 0; break;
					case SDLK_3: keys[3] = 0; break;
					case SDLK_q: keys[4] = 0; break;
					case SDLK_w: keys[5] = 0; break;
					case SDLK_e: keys[6] = 0; break;
					case SDLK_a: keys[7] = 0; break;
					case SDLK_s: keys[8] = 0; break;
					case SDLK_d: keys[9] = 0; break;
					case SDLK_z: keys[10] = 0; break;
					case SDLK_c: keys[11] = 0; break;
					case SDLK_4: keys[12] = 0; break;
					case SDLK_r: keys[13] = 0; break;
					case SDLK_f: keys[14] = 0; break;
					case SDLK_v: keys[15] = 0; break;
					default: break;
				}
				break;
			}
		}
	}

	return terminate;
}
