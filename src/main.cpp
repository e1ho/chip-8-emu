#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>

#include <iostream>
#include <thread>

#include "chip8.h"

int main(const int argc, char* argv[]) {
	if (argc != 2) {
		std::cerr << "usage: " << argv[0] << " <rom file>" << std::endl;
		return 1;
	}

	chip8 c8;
	c8.init();

	if (!c8.load_rom(argv[1])) {
		std::cerr << "failed to load rom: " << argv[1] << std::endl;
		return 1;
	}

	if (SDL_Init(SDL_INIT_EVERYTHING) < 0) {
		std::cerr << "sdl failed to initialize. error: " << SDL_GetError() << std::endl;
		return 1;
	}

	SDL_Window* window = SDL_CreateWindow("chip8", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, c8.screen_width * 10, c8.screen_height * 10, SDL_WINDOW_SHOWN);
	SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
	SDL_Texture* texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, c8.screen_width, c8.screen_height);

	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
	SDL_RenderClear(renderer);
	SDL_RenderPresent(renderer);

	while (true) {
		c8.input();
		c8.cycle();

		c8.draw(texture, renderer);

		std::this_thread::sleep_for(std::chrono::milliseconds(4));
	}
}
