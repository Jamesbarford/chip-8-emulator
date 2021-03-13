#include <sys/time.h>

#include "chip_8.h"
#include "display.h"

#define SCALE 10
#define DELAY 3
#define HEIGHT 32
#define WIDTH 64

int main(int argc, char **argv)
{
	if (argc != 2)
	{
		fprintf(stderr, "Must provide name of rom to run\n");
		exit(EXIT_FAILURE);
	}

	struct timeval stop, start;
	display_t *display = alloc_display("chip 8 emulator", WIDTH * SCALE, HEIGHT * SCALE, WIDTH, HEIGHT);
	char *rom_name = argv[1];
	chip_8_t *chip_8 = boot_chip8(rom_name);
	uint32_t pitch = sizeof(chip_8->video[0] * WIDTH);

	BOOL terminate = False;
	gettimeofday(&start, NULL);

	while (terminate == False)
	{
		terminate = handle_input(chip_8->keypad);

		gettimeofday(&stop, NULL);

		double difference = (stop.tv_sec - start.tv_sec) * 1000000 + stop.tv_usec - start.tv_usec;

		if (difference > DELAY)
		{
			start = stop;

			emulate_cycle(chip_8);
			update_display(display, chip_8->video, pitch);
		}
	}

	free_chip_8(chip_8);
	
	return 0;
}
