# chip-8 emulator

This project is a chip-8 emulator/interpreter implemented in C++ using SDL2 for graphics and input handling.

## Requirements

- C++11 or later
- SDL2 library

## Usage

To run the emulator, simply drag and drop a chip-8 ROM file (there are some already provided in the demos folder) onto the `chip8.exe` executable. This will automatically launch the emulator with the selected ROM.

## Controls

Chip-8 uses a 16-key hexadecimal keypad. This emulator maps those keys to your keyboard as follows:

```
1 2 3 4
Q W E R
A S D F
Z X C V
```

## To-do

- Implement sound
- Error handling
- Debug overlays

## Acknowledgements

- [How to write an emulator (CHIP-8 interpreter) by Laurence Muller](https://www.libsdl.org/)
- [Cowgod's Chip-8 Technical Reference](http://devernay.free.fr/hacks/chip8/C8TECH10.HTM)
- [dmatlack's ROMs and some opcode decoding logic](https://github.com/dmatlack/chip8)
- [SDL2 Library](https://www.libsdl.org/)
