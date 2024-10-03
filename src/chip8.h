#pragma once
#include <SDL2/SDL_render.h>
#include <array>

#include "opcode.h"

class chip8 {
public:
	static constexpr int screen_width = 64;
	static constexpr int screen_height = 32;
	static constexpr int window_scale = 10;

	chip8();
	~chip8();

	void init(SDL_Renderer* renderer);
	void cycle();					   // emulate a single cycle
	void draw(SDL_Renderer* renderer); // draw the graphics buffer to the screen
	void input();					   // handle input events
	void update_timers();			   // update the delay and sound timers

	bool load_rom(const char* filename);

	bool should_quit() const { return quit_; }

	uint8_t memory_[4096];  // 4k memory
	uint8_t v_[16];			// 16 general purpose registers
	uint16_t i_;			// index register
	uint16_t pc_;			// program counter

	uint16_t stack_[16];	// stack
	uint16_t sp_;			// stack pointer

	uint8_t gfx_[screen_width * 
		screen_height];		// graphics buffer

	uint8_t key_[16];		// keypad

	uint8_t delay_timer_;	// delay timer
	uint8_t sound_timer_;	// sound timer

	bool should_draw_;		// flag to indicate if the screen should be redrawn
private:
	// chip8 fontset
	static constexpr std::array<uint8_t, 80> fontset_ = {
		0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
		0x20, 0x60, 0x20, 0x20, 0x70, // 1
		0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
		0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
		0x90, 0x90, 0xF0, 0x10, 0x10, // 4
		0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
		0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
		0xF0, 0x10, 0x20, 0x40, 0x40, // 7
		0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
		0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
		0xF0, 0x90, 0xF0, 0x90, 0x90, // A
		0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
		0xF0, 0x80, 0x80, 0x80, 0xF0, // C
		0xE0, 0x90, 0x90, 0x90, 0xE0, // D
		0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
		0xF0, 0x80, 0xF0, 0x80, 0x80  // F
	};

	opcode opcode_handler_;
	uint16_t opcode_; // current opcode

	SDL_Texture* texture_; 

	bool quit_;	 // flag to indicate if the program should quit
};
