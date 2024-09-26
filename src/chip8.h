#pragma once
#include <SDL2/SDL_render.h>
#include <array>

class chip8 {
public:
	static constexpr int screen_width = 64;
	static constexpr int screen_height = 32;
	static constexpr int window_scale = 10;

	chip8();
	~chip8();

	void init();
	void cycle(); // emulate a single cycle
	void draw(SDL_Texture* texture, SDL_Renderer* renderer); // draw the graphics buffer to the screen
	void input(); // handle input events

	bool load_rom(const char* filename);

	bool should_quit() const { return quit; }
private:
	// chip8 fontset
	static constexpr std::array<uint8_t, 80> fontset = {
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

	unsigned short opcode;		// current opcode
	unsigned char memory[4096]; // 4k memory
	unsigned char V[16];		// 16 general purpose registers
	unsigned short I;			// index register
	unsigned short pc;			// program counter

	unsigned char gfx[screen_width * screen_height]; // graphics buffer
	
	unsigned short stack[16];	// stack
	unsigned short sp;			// stack pointer

	unsigned char key[16];		// keypad

	unsigned char delay_timer;	// delay timer
	unsigned char sound_timer;	// sound timer

	bool should_draw;			// flag to indicate if the screen should be redrawn
	bool quit;					// flag to indicate if the program should quit
};
