#pragma once
#include <functional>
#include <unordered_map>

class chip8;

struct decoded_opcode {
	uint8_t op;   // first nibble (opcode identifier)
	uint8_t x;    // second nibble (usually a register index)
	uint8_t y;    // third nibble (usually a register index)
	uint8_t n;    // fourth nibble (often used for small values)
	uint8_t nn;   // lower byte (used for immediate values)
	uint16_t nnn; // lower 12 bits (used for addresses)
};

class opcode {
public:
	using opcode_func = std::function<void(chip8&, const decoded_opcode&)>;
	struct opcode_entry {
		uint8_t opcode;
		opcode_func function;
	};

	opcode();

	void execute(chip8& c8, uint16_t opcode) const;
private:
	std::unordered_map<uint8_t, opcode_func> main_table_;
	std::unordered_map<uint8_t, opcode_func> table0_;
	std::unordered_map<uint8_t, opcode_func> table8_;
	std::unordered_map<uint8_t, opcode_func> table_e_;
	std::unordered_map<uint16_t, opcode_func> table_f_;

	void init();

	// helper functions
	static opcode_func table_dispatch(const std::unordered_map<uint8_t, opcode_func>& table, std::function<uint8_t(const decoded_opcode&)> extractor);
	static opcode_func table_dispatch(const std::unordered_map<uint16_t, opcode_func>& table, std::function<uint16_t(const decoded_opcode&)> extractor);

	static void skip_next_instruction(chip8& c8);
	static void exec_next_instruction(chip8& c8);

	// opcode implementations
	static void op_00E0(chip8& c8, decoded_opcode decoded);
	static void op_00EE(chip8& c8, decoded_opcode decoded);
	static void op_1nnn(chip8& c8, decoded_opcode decoded);
	static void op_2nnn(chip8& c8, decoded_opcode decoded);
	static void op_3xnn(chip8& c8, decoded_opcode decoded);
	static void op_4xnn(chip8& c8, decoded_opcode decoded);
	static void op_5xy0(chip8& c8, decoded_opcode decoded);
	static void op_6xnn(chip8& c8, decoded_opcode decoded);
	static void op_7xnn(chip8& c8, decoded_opcode decoded);
	static void op_8xy0(chip8& c8, decoded_opcode decoded);
	static void op_8xy1(chip8& c8, decoded_opcode decoded);
	static void op_8xy2(chip8& c8, decoded_opcode decoded);
	static void op_8xy3(chip8& c8, decoded_opcode decoded);
	static void op_8xy4(chip8& c8, decoded_opcode decoded);
	static void op_8xy5(chip8& c8, decoded_opcode decoded);
	static void op_8xy6(chip8& c8, decoded_opcode decoded);
	static void op_8xy7(chip8& c8, decoded_opcode decoded);
	static void op_8xyE(chip8& c8, decoded_opcode decoded);
	static void op_9xy0(chip8& c8, decoded_opcode decoded);
	static void op_Annn(chip8& c8, decoded_opcode decoded);
	static void op_Bnnn(chip8& c8, decoded_opcode decoded);
	static void op_Cxnn(chip8& c8, decoded_opcode decoded);
	static void op_Dxyn(chip8& c8, decoded_opcode decoded);
	static void op_Ex9E(chip8& c8, decoded_opcode decoded);
	static void op_ExA1(chip8& c8, decoded_opcode decoded);
	static void op_Fx07(chip8& c8, decoded_opcode decoded);
	static void op_Fx0A(chip8& c8, decoded_opcode decoded);
	static void op_Fx15(chip8& c8, decoded_opcode decoded);
	static void op_Fx18(chip8& c8, decoded_opcode decoded);
	static void op_Fx1E(chip8& c8, decoded_opcode decoded);
	static void op_Fx29(chip8& c8, decoded_opcode decoded);
	static void op_Fx33(chip8& c8, decoded_opcode decoded);
	static void op_Fx55(chip8& c8, decoded_opcode decoded);
	static void op_Fx65(chip8& c8, decoded_opcode decoded);
};