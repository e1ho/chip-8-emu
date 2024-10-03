#include <random>

#include "opcode.h"
#include "chip8.h"

// thread_local ensures that each thread has its own instance of the random number generator
thread_local std::mt19937 generator(std::random_device{}());

opcode::opcode() {
    init();
}

decoded_opcode decode_opcode(const uint16_t opcode) {
    decoded_opcode decoded;

    decoded.op = (opcode & 0xF000) >> 12;
    decoded.x = (opcode & 0x0F00) >> 8;
    decoded.y = (opcode & 0x00F0) >> 4;
    decoded.n = opcode & 0x000F;
    decoded.nn = opcode & 0x00FF;
    decoded.nnn = opcode & 0x0FFF;

    return decoded;
}

void opcode::execute(chip8& c8, const uint16_t opcode) const {
    const decoded_opcode decoded = decode_opcode(opcode);
    const uint8_t index = decoded.op;

    const auto it = main_table_.find(index);
    if (it != main_table_.end())
        it->second(c8, decoded);
}

void opcode::init() {
    // initialize main table
    main_table_ = {
		{ 0x0, table_dispatch(table0_, [](const decoded_opcode& decoded) { return decoded.nn; }) },
        { 0x1, op_1nnn },
		{ 0x2, op_2nnn },
		{ 0x3, op_3xnn },
		{ 0x4, op_4xnn },
		{ 0x5, op_5xy0 },
		{ 0x6, op_6xnn },
		{ 0x7, op_7xnn },
		{ 0x8, table_dispatch(table8_, [](const decoded_opcode& decoded) { return decoded.n; }) },
        { 0x9, op_9xy0 },
		{ 0xA, op_Annn },
		{ 0xB, op_Bnnn },
		{ 0xC, op_Cxnn },
		{ 0xD, op_Dxyn },
		{ 0xE, table_dispatch(table_e_, [](const decoded_opcode& decoded) { return decoded.nn; }) },
        { 0xF, table_dispatch(table_f_, [](const decoded_opcode& decoded) { return decoded.nn; }) },
    };

    // initialize sub tables
    table0_ = {
        { 0xE0, op_00E0 },
        { 0xEE, op_00EE }
    };

    table8_ = {
        { 0x0, op_8xy0 },
        { 0x1, op_8xy1 },
        { 0x2, op_8xy2 },
        { 0x3, op_8xy3 },
        { 0x4, op_8xy4 },
        { 0x5, op_8xy5 },
        { 0x6, op_8xy6 },
        { 0x7, op_8xy7 },
        { 0xE, op_8xyE }
    };

    table_e_ = {
        { 0x9E, op_Ex9E },
        { 0xA1, op_ExA1 }
    };

    table_f_ = {
        { 0x07, op_Fx07 },
        { 0x0A, op_Fx0A },
        { 0x15, op_Fx15 },
        { 0x18, op_Fx18 },
        { 0x1E, op_Fx1E },
        { 0x29, op_Fx29 },
        { 0x33, op_Fx33 },
        { 0x55, op_Fx55 },
        { 0x65, op_Fx65 }
    };
}

// helper functions
opcode::opcode_func opcode::table_dispatch(const std::unordered_map<uint8_t, opcode_func>& table, std::function<uint8_t(const decoded_opcode&)> extractor) {
    return [&table, extractor](chip8& c8, const decoded_opcode& decoded) {
	    const uint8_t key = extractor(decoded);
	    const auto it = table.find(key);
        if (it != table.end()) {
            it->second(c8, decoded);
        }
        };
}

opcode::opcode_func opcode::table_dispatch(const std::unordered_map<uint16_t, opcode_func>& table, std::function<uint16_t(const decoded_opcode&)> extractor) {
    return [&table, extractor](chip8& c8, const decoded_opcode& decoded) {
        const uint16_t key = extractor(decoded);
        const auto it = table.find(key);
        if (it != table.end()) {
            it->second(c8, decoded);
        }
        };
}

void opcode::skip_next_instruction(chip8& c8) {
    c8.pc_ += 4;
}

void opcode::exec_next_instruction(chip8& c8) {
    c8.pc_ += 2;
}

// clear display
void opcode::op_00E0(chip8& c8, decoded_opcode decoded) {
    std::fill(std::begin(c8.gfx_), std::end(c8.gfx_), 0);
    c8.should_draw_ = true;
    exec_next_instruction(c8);
}

// return from subroutine
void opcode::op_00EE(chip8& c8, decoded_opcode decoded) {
    c8.sp_--;
    c8.pc_ = c8.stack_[c8.sp_];
    exec_next_instruction(c8);
}

// jump to address nnn
void opcode::op_1nnn(chip8& c8, decoded_opcode decoded) {
    c8.pc_ = decoded.nnn;
}

// call subroutine at nnn
void opcode::op_2nnn(chip8& c8, decoded_opcode decoded) {
    c8.stack_[c8.sp_] = c8.pc_;
    c8.sp_++;
    c8.pc_ = decoded.nnn;
}

// skip next instruction if Vx == nn
void opcode::op_3xnn(chip8& c8, decoded_opcode decoded) {
    if (c8.v_[decoded.x] == decoded.nn)
        skip_next_instruction(c8);
    else
        exec_next_instruction(c8);
}

// skip next instruction if Vx != nn
void opcode::op_4xnn(chip8& c8, decoded_opcode decoded) {
    if (c8.v_[decoded.x] != decoded.nn)
        skip_next_instruction(c8);
    else
        exec_next_instruction(c8);
}

// skip next instruction if Vx == Vy
void opcode::op_5xy0(chip8& c8, decoded_opcode decoded) {
    if (c8.v_[decoded.x] == c8.v_[decoded.y])
        skip_next_instruction(c8);
    else
        exec_next_instruction(c8);
}

// set Vx = nn
void opcode::op_6xnn(chip8& c8, decoded_opcode decoded) {
    c8.v_[decoded.x] = decoded.nn;
    exec_next_instruction(c8);
}

// set Vx = Vx + nn
void opcode::op_7xnn(chip8& c8, decoded_opcode decoded) {
    c8.v_[decoded.x] += decoded.nn;
    exec_next_instruction(c8);
}

// set Vx = Vy
void opcode::op_8xy0(chip8& c8, decoded_opcode decoded) {
    c8.v_[decoded.x] = c8.v_[decoded.y];
    exec_next_instruction(c8);
}

// set Vx = Vx OR Vy
void opcode::op_8xy1(chip8& c8, decoded_opcode decoded) {
    c8.v_[decoded.x] |= c8.v_[decoded.y];
    exec_next_instruction(c8);
}

// set Vx = Vx AND Vy
void opcode::op_8xy2(chip8& c8, decoded_opcode decoded) {
    c8.v_[decoded.x] &= c8.v_[decoded.y];
    exec_next_instruction(c8);
}

// set Vx = Vx XOR Vy
void opcode::op_8xy3(chip8& c8, decoded_opcode decoded) {
    c8.v_[decoded.x] ^= c8.v_[decoded.y];
    exec_next_instruction(c8);
}

// set Vx = Vx + Vy, set VF = carry
void opcode::op_8xy4(chip8& c8, decoded_opcode decoded) {
    uint16_t sum = c8.v_[decoded.x] + c8.v_[decoded.y];
    c8.v_[0xF] = (sum > 0xFF) ? 1 : 0; // set the carry flag
    c8.v_[decoded.x] = sum & 0xFF; // keep only the lowest 8 bits
    exec_next_instruction(c8);
}

// set Vx = Vx - Vy, set VF = NOT borrow
void opcode::op_8xy5(chip8& c8, decoded_opcode decoded) {
    c8.v_[0xF] = (c8.v_[decoded.x] > c8.v_[decoded.y]) ? 1 : 0;
    c8.v_[decoded.x] -= c8.v_[decoded.y];
    exec_next_instruction(c8);
}

// set Vx = Vx SHR 1 (shift right by 1)
void opcode::op_8xy6(chip8& c8, decoded_opcode decoded) {
    c8.v_[0xF] = c8.v_[decoded.x] & 0x1;
    c8.v_[decoded.x] >>= 1;
    exec_next_instruction(c8);
}

// set Vx = Vy - Vx, set VF = NOT borrow
void opcode::op_8xy7(chip8& c8, decoded_opcode decoded) {
    c8.v_[0xF] = (c8.v_[decoded.y] > c8.v_[decoded.x]) ? 1 : 0;
    c8.v_[decoded.x] = c8.v_[decoded.y] - c8.v_[decoded.x];
    exec_next_instruction(c8);
}

// set Vx = Vx SHL 1 (shift left by 1)
void opcode::op_8xyE(chip8& c8, decoded_opcode decoded) {
    c8.v_[0xF] = c8.v_[decoded.x] >> 7;
    c8.v_[decoded.x] <<= 1;
    exec_next_instruction(c8);
}

// skip next instruction if Vx != Vy
void opcode::op_9xy0(chip8& c8, decoded_opcode decoded) {
    if (c8.v_[decoded.x] != c8.v_[decoded.y])
        skip_next_instruction(c8);
    else
        exec_next_instruction(c8);
}

// set I = nnn
void opcode::op_Annn(chip8& c8, const decoded_opcode decoded) {
    c8.i_ = decoded.nnn;
    exec_next_instruction(c8);
}

// jump to location nnn + V0
void opcode::op_Bnnn(chip8& c8, const decoded_opcode decoded) {
    c8.pc_ = decoded.nnn + c8.v_[0];
}

// set Vx = random byte AND nn
void opcode::op_Cxnn(chip8& c8, const decoded_opcode decoded) {
    std::uniform_int_distribution<unsigned int> distribution(0, 0xFF);
    c8.v_[decoded.x] = static_cast<uint8_t>(distribution(generator)) & decoded.nn;
    exec_next_instruction(c8);
}

// display n-byte sprite starting at memory location I at (Vx, Vy), set VF = collision
void opcode::op_Dxyn(chip8& c8, const decoded_opcode decoded) {
    const uint8_t x_coord = c8.v_[decoded.x] % chip8::screen_width;
    const uint8_t y_coord = c8.v_[decoded.y] % chip8::screen_height;
    const uint8_t height = decoded.n;

    c8.v_[0xF] = 0; // reset collision flag

    // iterate over each row of the sprite
    for (uint8_t row = 0; row < height; row++) {
		const uint8_t sprite_byte = c8.memory_[c8.i_ + row];

        // iterate over each column of the sprite
		for (uint8_t col = 0; col < 8; col++) {
			const uint8_t sprite_pixel = sprite_byte & (0x80 >> col);
			if (sprite_pixel) {
				const uint8_t x = (x_coord + col) % chip8::screen_width;
                const uint8_t y = (y_coord + row) % chip8::screen_height;
                const size_t index = x + (y * chip8::screen_width);

                // check for collision/if the pixel has already been set
                if (c8.gfx_[index] == 1)
					c8.v_[0xF] = 1; // set collision flag

                // draw the pixel
                c8.gfx_[index] ^= 1;
			}
		}
	}

    c8.should_draw_ = true;
    exec_next_instruction(c8);
}

// skip next instruction if key with the value of Vx is pressed
void opcode::op_Ex9E(chip8& c8, const decoded_opcode decoded) {
    if (c8.key_[c8.v_[decoded.x]] != 0)
        skip_next_instruction(c8);
    else
        exec_next_instruction(c8);
}

// skip next instruction if key with the value of Vx is not pressed
void opcode::op_ExA1(chip8& c8, const decoded_opcode decoded) {
    if (c8.key_[c8.v_[decoded.x]] == 0)
        skip_next_instruction(c8);
    else
        exec_next_instruction(c8);
}

// set Vx = delay timer value
void opcode::op_Fx07(chip8& c8, const decoded_opcode decoded) {
    c8.v_[decoded.x] = c8.delay_timer_;
    exec_next_instruction(c8);
}

// wait for a key press, store the value of the key in Vx
// if there is no key press, the program counter doesn't change and the instruction is repeated
void opcode::op_Fx0A(chip8& c8, const decoded_opcode decoded) {
    for (uint8_t i = 0; i < 16; ++i) {
        if (c8.key_[i] != 0) {
            c8.v_[decoded.x] = i;
            exec_next_instruction(c8);
            return;
        }
    }
}

// set delay timer = Vx
void opcode::op_Fx15(chip8& c8, const decoded_opcode decoded) {
    c8.delay_timer_ = c8.v_[decoded.x];
    exec_next_instruction(c8);
}

// set sound timer = Vx
void opcode::op_Fx18(chip8& c8, const decoded_opcode decoded) {
    c8.sound_timer_ = c8.v_[decoded.x];
    exec_next_instruction(c8);
}

// set I = I + Vx
void opcode::op_Fx1E(chip8& c8, const decoded_opcode decoded) {
    c8.i_ += c8.v_[decoded.x];
    exec_next_instruction(c8);
}

// set I = location of sprite for digit Vx
void opcode::op_Fx29(chip8& c8, const decoded_opcode decoded) {
    c8.i_ = c8.v_[decoded.x] * 5; // each sprite is 5 bytes long
    exec_next_instruction(c8);
}

// store BCD representation of Vx in memory locations I, I+1, and I+2
void opcode::op_Fx33(chip8& c8, const decoded_opcode decoded) {
	const uint8_t value = c8.v_[decoded.x];
    c8.memory_[c8.i_] = value / 100;
    c8.memory_[c8.i_ + 1] = (value / 10) % 10;
    c8.memory_[c8.i_ + 2] = value % 10;
    exec_next_instruction(c8);
}

// store registers V0 through Vx in memory starting at location I
void opcode::op_Fx55(chip8& c8, const decoded_opcode decoded) {
	const uint8_t x = decoded.x;
    for (int i = 0; i <= x; i++)
        c8.memory_[c8.i_ + i] = c8.v_[i];

    exec_next_instruction(c8);
}

// read registers V0 through Vx from memory starting at location I
void opcode::op_Fx65(chip8& c8, const decoded_opcode decoded) {
	const uint8_t x = decoded.x;
    for (int i = 0; i <= x; i++)
        c8.v_[i] = c8.memory_[c8.i_ + i];

    exec_next_instruction(c8);
}
