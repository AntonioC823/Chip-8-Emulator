#pragma once
#include <string>
#include <vector>
#include <SFML/Graphics.hpp>

const int DISPLAY_WIDTH = 64;
const int DISPLAY_HEIGHT = 32;

class Chip8
{
	// Public methods
public:
	Chip8();
	void loadROM(const std::string& fileName);
	void cycle();
	std::vector< sf::RectangleShape> printDisplay() const;
	void pressKey(uint8_t key);
	void releaseKey(uint8_t key);
	bool isSoundActive() const;

	// Private methods
private:
	uint16_t fetch();
	void decodeAndExecute();
	void updateTimers();

	// Private atributes
private:
	// Registers
	uint8_t V[16];					// array for the 16 registers (8-bit)
	uint16_t I;						// additional index register used to store memory addresses
	uint16_t programCounter;		// psuedo-register that stores the currently executing address
	uint8_t stackPointer;			// psuedo-register used to point to the topmost level of the stack
	uint16_t opcode;				// current fetched instruction

	// Memory
	uint8_t memory[4096];			// memory array for 4KB

	// Stack
	uint16_t stack[16];				// stack array for 16 two-byte entries

	// Timers
	uint8_t delayTimer;				// if above 0, decremented at 60Hz (60 times per second)
	uint8_t soundTimer;				// if above 0, decremented at 60Hz (60 times per second)

	// Font
	static constexpr uint8_t font[80] =
	{
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

	// Display
	bool display[DISPLAY_WIDTH * DISPLAY_HEIGHT];			// Represents the CHIP-8 display (1 byte per pixel, using 64x32 resolution)

	// Keypad
	uint8_t keypad[16];				// will map to 16 keys on qwerty layout
};

