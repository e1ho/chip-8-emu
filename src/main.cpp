#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>

#include <iostream>
#include <thread>

#include "chip8.h"

int main(const int argc, char* argv[]) {
	if (argc != 2)
		return 1;

	if (SDL_Init(SDL_INIT_EVERYTHING) < 0)
		return 1;

	SDL_Window* window = SDL_CreateWindow("chip-8 emulator",
		SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
		chip8::screen_width * chip8::window_scale,
		chip8::screen_height * chip8::window_scale,
		SDL_WINDOW_SHOWN);
	if (!window) {
		SDL_Quit();
		return 1;
	}

	SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
	if (!renderer) {
		SDL_DestroyWindow(window);
		SDL_Quit();
		return 1;
	}

	chip8 c8;
	c8.init(renderer);

	if (!c8.load_rom(argv[1])) {
		SDL_DestroyRenderer(renderer);
		SDL_DestroyWindow(window);
		SDL_Quit();
		return 1;
	}

	while (!c8.should_quit()) {
		auto frame_start = std::chrono::high_resolution_clock::now();

		c8.input();

		for (int i = 0; i < 10; ++i)
			c8.cycle(); // emulate 10 cycles per frame

		c8.update_timers(); // update timers at 60hz

		c8.draw(renderer);

		// calculate time spent processing this frame
		auto frame_end = std::chrono::high_resolution_clock::now();
		auto frame_duration = std::chrono::duration_cast<std::chrono::microseconds>(frame_end - frame_start);

		// sleep for the remaining time to maintain 60fps
		std::this_thread::sleep_for(std::chrono::microseconds(16667) - frame_duration);
	}

	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();

	return 0;
}
