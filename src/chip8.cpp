#include <SDL2/SDL_events.h>

#include <array>
#include <iostream>
#include <fstream>

#include "chip8.h"

chip8::chip8() = default;
chip8::~chip8() = default;

// initialize registers and memory once
void chip8::init() {
	pc = 0x200; // program counter starts at 0x200
	opcode = 0; // reset current opcode
	I = 0;		// reset index register
	sp = 0;		// reset stack pointer

	// clear display
	for (int i = 0; i < 64 * 32; ++i) 
		gfx[i] = 0;

	// clear stack, V, and key registers
	for (int i = 0; i < 16; ++i) {
		stack[i] = 0;
		V[i] = 0;
		key[i] = 0;
	}

	// clear memory
	for (int i = 0; i < 4096; ++i) 
		memory[i] = 0;

	// load fontset
	for (int i = 0; i < 80; ++i) 
		memory[i] = fontset[i];

	// reset timers
	delay_timer = 0;
	sound_timer = 0;
}

void chip8::cycle() {
	// fetch opcode
	opcode = memory[pc] << 8 | memory[pc + 1];

	// decode opcode
	switch (opcode & 0xF000) {
	case 0x0000:
		switch (opcode & 0x00FF) {
		case 0x00E0: // 00E0: clear screen
			for (int i = 0; i < 64 * 32; ++i)
				gfx[i] = 0;
			draw_flag = true;
			pc += 2;
			break;
		case 0x00EE: // 00EE: return from subroutine
			--sp;
			pc = stack[sp];
			pc += 2;
			break;
		default:
			std::cerr << "unknown opcode [0x0000]: 0x" << std::hex << opcode << std::endl;
			pc += 2;
			break;
		}
		break;
	case 0x1000: // 1NNN: jump to address NNN
		pc = opcode & 0x0FFF;
		break;
	case 0x2000: // 2NNN: call subroutine at NNN
		stack[sp] = pc;
		++sp;
		pc = opcode & 0x0FFF;
		break;
	case 0x3000: // 3XNN: skip next instruction if VX == NN
		if (V[(opcode & 0x0F00) >> 8] == (opcode & 0x00FF))
			pc += 4;
		else
			pc += 2;
		break;
	case 0x4000: // 4XNN: skip next instruction if VX != NN
		if (V[(opcode & 0x0F00) >> 8] != (opcode & 0x00FF))
			pc += 4;
		else
			pc += 2;
		break;
	case 0x5000: // 5XY0: skip next instruction if VX == VY
		if (V[(opcode & 0x0F00) >> 8] == V[(opcode & 0x00F0) >> 4])
			pc += 4;
		else
			pc += 2;
		break;
	case 0x6000: // 6XNN: set VX to NN
		V[(opcode & 0x0F00) >> 8] = opcode & 0x00FF;
		pc += 2;
		break;
	case 0x7000: // 7XNN: add NN to VX
		V[(opcode & 0x0F00) >> 8] += opcode & 0x00FF;
		pc += 2;
		break;
	case 0x8000:
		switch (opcode & 0x000F) {
			// 8XY0: set VX to VY
		case 0x0000:
			V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x00F0) >> 4];
			pc += 2;
			break;
		case 0x0001: // 8XY1: set VX to VX | VY
			V[(opcode & 0x0F00) >> 8] |= V[(opcode & 0x00F0) >> 4];
			pc += 2;
			break;
		case 0x0002: // 8XY2: set VX to VX & VY
			V[(opcode & 0x0F00) >> 8] &= V[(opcode & 0x00F0) >> 4];
			pc += 2;
			break;
		case 0x0003: // 8XY3: set VX to VX ^ VY
			V[(opcode & 0x0F00) >> 8] ^= V[(opcode & 0x00F0) >> 4];
			pc += 2;
			break;
		case 0x0004: // 8XY4: add VY to VX, set VF to 1
			if (V[(opcode & 0x00F0) >> 4] > (0xFF - V[(opcode & 0x0F00) >> 8]))
				V[0xF] = 1;
			else
				V[0xF] = 0;

			V[(opcode & 0x0F00) >> 8] += V[(opcode & 0x00F0) >> 4];
			pc += 2;
			break;
		case 0x0005: // 8XY5: subtract VY from VX, set VF to 0 if borrow
			if (V[(opcode & 0x00F0) >> 4] > V[(opcode & 0x0F00) >> 8])
				V[0xF] = 0;
			else
				V[0xF] = 1;

			V[(opcode & 0x0F00) >> 8] -= V[(opcode & 0x00F0) >> 4];
			pc += 2;
			break;
		case 0x0006: // 8XY6: shift VX right by 1, set VF to least significant bit of VX
			V[0xF] = V[(opcode & 0x0F00) >> 8] & 0x1;
			V[(opcode & 0x0F00) >> 8] >>= 1;
			pc += 2;
			break;
		case 0x0007: // 8XY7: set VX to VY - VX, set VF to 0 if borrow
			if (V[(opcode & 0x0F00) >> 8] > V[(opcode & 0x00F0) >> 4])
				V[0xF] = 0;
			else
				V[0xF] = 1;
			break;
		case 0x000E: // 8XYE: shift VX left by 1, set VF to most significant bit of VX
			V[0xF] = V[(opcode & 0x0F00) >> 8] >> 7;
			V[(opcode & 0x0F00) >> 8] <<= 1;
			pc += 2;
			break;
		default:
			std::cerr << "unknown opcode [0x8000]: 0x" << std::hex << opcode << std::endl;
			break;
		}
		break;
	case 0x9000:
		if (V[(opcode & 0x0F00) >> 8] != V[(opcode & 0x00F0) >> 4])
			pc += 4;
		else
			pc += 2;
		break;
	case 0xA000: // ANNN: set I to address NNN
		I = opcode & 0x0FFF;
		pc += 2;
		break;
	case 0xB000: // BNNN: jump to address NNN + V0
		pc = (opcode & 0x0FFF) + V[0];
		break;
	case 0xC000: // CXNN: set VX to random byte & NN
		V[(opcode & 0x0F00) >> 8] = (rand() % 0xFF) & (opcode & 0x00FF);
		pc += 2;
		break;
	case 0xD000:
	{
		unsigned const short x = V[(opcode & 0x0F00) >> 8];
		unsigned const short y = V[(opcode & 0x00F0) >> 4];
		unsigned const short height = opcode & 0x000F;

		V[0xF] = 0;
		for (int yline = 0; yline < height; yline++) {
			unsigned short pixel = memory[I + yline];
			for (int xline = 0; xline < 8; xline++) {
				if ((pixel & (0x80 >> xline)) != 0) {
					if (gfx[(x + xline + ((y + yline) * 64))] == 1)
						V[0xF] = 1;
					gfx[x + xline + ((y + yline) * 64)] ^= 1;
				}
			}
		}

		draw_flag = true; 
		pc += 2;
	}
	break;
	case 0xE000:
		switch (opcode & 0x00FF) {
		case 0x009E: // EX9E: skip next instruction if key with value VX is pressed
			if (key[V[(opcode & 0x0F00) >> 8]] != 0)				
				pc += 4;
			else
				pc += 2;
			break;
		case 0x00A1: // EXA1: skip next instruction if key with value VX is not pressed
			if (key[V[(opcode & 0x0F00) >> 8]] == 0)				
				pc += 4;
			else
				pc += 2;
			break;
		default:
			std::cerr << "unknown opcode [0xE000]: 0x" << std::hex << opcode << std::endl;
		}
		break;
	case 0xF000:
		switch (opcode & 0x00FF) {
		case 0x0007: // FX07: set VX to value of delay timer
			V[(opcode & 0x0F00) >> 8] = delay_timer;
			pc += 2;
			break;
		case 0x000A: // FX0A: wait for key press, store value in VX
		{
			bool key_pressed = false;

			for (int i = 0; i < 16; ++i) {
				if (key[i] != 0) {
					V[(opcode & 0x0F00) >> 8] = i;
					key_pressed = true;
					break;
				}
			}

			if (!key_pressed)
				return; // skip pc increment to effectively pause execution

			pc += 2;
		}
			break;
		case 0x0015: // FX15: set delay timer to VX
			delay_timer = V[(opcode & 0x0F00) >> 8];
			pc += 2;
			break;
		case 0x0018: // FX18: set sound timer to VX
			sound_timer = V[(opcode & 0x0F00) >> 8];
			pc += 2;
			break;
		case 0x001E: // FX1E: add VX to I
			if (I + V[(opcode & 0x0F00) >> 8] > 0xFFF)
				V[0xF] = 1;
			else
				V[0xF] = 0;

			I += V[(opcode & 0x0F00) >> 8];
			pc += 2;
			break;
		case 0x0029: // FX29: set I to location of sprite for digit VX
			I = V[(opcode & 0x0F00) >> 8] * 0x5;
			pc += 2;
			break;
		case 0x0033: // FX33: store BCD representation of VX in memory locations I, I+1, I+2
			memory[I] = V[(opcode & 0x0F00) >> 8] / 100;
			memory[I + 1] = (V[(opcode & 0x0F00) >> 8] / 10) % 10;
			memory[I + 2] = V[(opcode & 0x0F00) >> 8] % 10;
			pc += 2;
			break;
		case 0x0055: // FX55: store V0 to VX in memory starting at address I
			for (int i = 0; i <= ((opcode & 0x0F00) >> 8); ++i)
				memory[I + i] = V[i];

			I += ((opcode & 0x0F00) >> 8) + 1;
			pc += 2;
			break;
		case 0x0065: // FX65: fill V0 to VX with values from memory starting at address I
			for (int i = 0; i <= ((opcode & 0x0F00) >> 8); ++i)
				V[i] = memory[I + i];

			I += ((opcode & 0x0F00) >> 8) + 1;
			pc += 2;
			break;
		default:
			std::cerr << "unknown opcode [0xF000]: 0x" << std::hex << opcode << std::endl;
			break;
		}
		break;
	default:
		std::cerr << "unknown opcode: 0x" << std::hex << opcode << std::endl;
		break;
	}

	if (delay_timer > 0)
		--delay_timer;

	// todo: add sound
	if (sound_timer > 0)
		--sound_timer;
}

void chip8::draw(SDL_Texture* texture, SDL_Renderer* renderer) {
	if (draw_flag) {
		int scale = 10;
		uint32_t pixels[screen_width * screen_height];

		for (int y = 0; y < screen_height; ++y) {
			for (int x = 0; x < screen_width; ++x) {
				int i = x + (y * screen_width);
				if (gfx[i] == 0)
					pixels[i] = 0xFF000000; // black
				else
					pixels[i] = 0xFFFFFFFF; // white
			}
		}

		SDL_UpdateTexture(texture, nullptr, pixels, screen_width * sizeof(uint32_t));
		SDL_RenderClear(renderer);
		SDL_Rect destRect = {0, 0, screen_width * scale, screen_height * scale};
		SDL_RenderCopy(renderer, texture, nullptr, &destRect);
		SDL_RenderPresent(renderer);

		// reset the draw flag
		draw_flag = false;
	}
}

SDL_Keycode keymap[16] = {
	SDLK_x,  // 0
	SDLK_1,  // 1
	SDLK_2,  // 2
	SDLK_3,  // 3
	SDLK_q,  // 4
	SDLK_w,  // 5
	SDLK_e,  // 6
	SDLK_a,  // 7
	SDLK_s,  // 8
	SDLK_d,  // 9
	SDLK_z,  // A
	SDLK_c,  // B
	SDLK_4,  // C
	SDLK_r,  // D
	SDLK_f,  // E
	SDLK_v,  // F
};

void chip8::input() {
	SDL_Event e;

	while (SDL_PollEvent(&e)) {
		if (e.type == SDL_QUIT)
			exit(0);

		if (e.type == SDL_KEYDOWN) {
			if (e.key.keysym.sym == SDLK_ESCAPE)
				exit(0);

			for (int i = 0; i < 16; ++i) {
				if (e.key.keysym.sym == keymap[i]) {
					key[i] = 1;
				}
			}
		}

		if (e.type == SDL_KEYUP) {
			for (int i = 0; i < 16; ++i) {
				if (e.key.keysym.sym == keymap[i]) {
					key[i] = 0;
				}
			}
		}
	}
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
		memory[0x200 + i] = buffer[i];

	delete[] buffer;
	return true;
}
