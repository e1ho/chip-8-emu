#include <SDL2/SDL_events.h>

#include <array>
#include <iostream>
#include <fstream>

#include "chip8.h"

chip8::chip8() = default;
chip8::~chip8() {
	SDL_DestroyTexture(front_texture_);
	SDL_DestroyTexture(back_texture_);
}

void chip8::init(SDL_Renderer* renderer) {
	pc_ = 0x200; // program counter starts at 0x200
	opcode_ = 0; // reset current opcode
	i_ = 0;		 // reset index register
	sp_ = 0;	 // reset stack pointer

	// clear display
	std::fill(std::begin(gfx_), std::end(gfx_), 0);

	// clear stack, v, and key registers
	std::fill(std::begin(stack_), std::end(stack_), 0);
	std::fill(std::begin(v_), std::end(v_), 0);
	std::fill(std::begin(key_), std::end(key_), 0);

	// clear memory
	std::fill(std::begin(memory_), std::end(memory_), 0);

	// load fontset into memory
	std::copy(std::begin(fontset_), std::end(fontset_), std::begin(memory_));

	front_buffer_.fill(0xFF000000); // set all pixels to black
	back_buffer_.fill(0xFF000000);

	// create textures for double buffering
	front_texture_ = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, 
		SDL_TEXTUREACCESS_STREAMING, screen_width, screen_height);
	back_texture_ = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, 
		SDL_TEXTUREACCESS_STREAMING, screen_width, screen_height);

	// reset timers and flags
	delay_timer_ = 0;
	sound_timer_ = 0;
	should_draw_ = false;
}

void chip8::cycle() {
	// fetch opcode
	opcode_ = memory_[pc_] << 8 | memory_[pc_ + 1];

	// decode opcode and execute
	opcode_handler_.execute(*this, opcode_);
}

void chip8::draw(SDL_Renderer* renderer) {
	if (should_draw_) {
		// copy the gfx buffer to the back buffer
		for (int i = 0; i < screen_width * screen_height; ++i)
			back_buffer_[i] = gfx_[i] ? 0xFFFFFFFF : 0xFF000000;

		// update the texture with new pixel data, clear the renderer, and render the texture
		SDL_UpdateTexture(back_texture_, nullptr, back_buffer_.data(), screen_width * sizeof(uint32_t));
		SDL_RenderClear(renderer);
		SDL_RenderCopy(renderer, back_texture_, nullptr, nullptr);
		SDL_RenderPresent(renderer);

		// swap buffers and textures for double buffering
		std::swap(front_buffer_, back_buffer_);
		std::swap(front_texture_, back_texture_);

		// reset the draw flag
		should_draw_ = false;
	}
}

void chip8::input() {
	SDL_Event e;

	// map the chip-8 keypad to keyboard keys
	static constexpr std::array<SDL_Keycode, 16> keymap = {
		SDLK_x, SDLK_1, SDLK_2, SDLK_3,
		SDLK_q, SDLK_w, SDLK_e, SDLK_a,
		SDLK_s, SDLK_d, SDLK_z, SDLK_c,
		SDLK_4, SDLK_r, SDLK_f, SDLK_v
	};

	while (SDL_PollEvent(&e)) {
		if (e.type == SDL_QUIT)
			quit_ = true;

		if (e.type == SDL_KEYDOWN) {
			if (e.key.keysym.sym == SDLK_ESCAPE)
				quit_ = true;

			// set key state to pressed 
			for (int i = 0; i < 16; ++i)
				if (e.key.keysym.sym == keymap[i])
					key_[i] = 1;
		}

		if (e.type == SDL_KEYUP) {
			// set key state to released
			for (int i = 0; i < 16; ++i)
				if (e.key.keysym.sym == keymap[i])
					key_[i] = 0;
		}
	}
}

void chip8::update_timers() {
	if (delay_timer_ > 0)
		--delay_timer_;

	// todo: implement sound
	if (sound_timer_ > 0)
		--sound_timer_;
}

bool chip8::load_rom(const char* filename) {
	// open file in binary mode 
	std::ifstream file(filename, std::ios::binary | std::ios::ate);
	if (!file.is_open()) {
		std::cerr << "failed to open file: " << filename << std::endl;
		return false;
	}

	// get size of file
	const std::streampos size = file.tellg();
	if (size > (4096 - 512)) {
		std::cerr << "rom file too large: " << filename << std::endl;
		return false;
	}

	// allocate memory to hold rom and read it in
	const auto buffer = new char[size];
	file.seekg(0, std::ios::beg);
	file.read(buffer, size);
	file.close();

	// load the rom into memory starting at 0x200
	for (int i = 0; i < size; ++i)
		memory_[0x200 + i] = buffer[i];

	delete[] buffer;
	return true;
}
