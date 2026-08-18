#include "Chip8.h"
#include <fstream>
#include <iostream>
#include <cstdlib>
#include <chrono>

static constexpr uint16_t FONT_START = 0x000;
static constexpr uint16_t ROM_START = 0x200;
const unsigned short int PIXEL_SIZE = 10;		// Will represent the width and height of each pixel

Chip8::Chip8()
{
	// Initialize registers
	for (int i = 0; i < 16; i++)
	{
		V[i] = 0;
	}
	I = 0;
	programCounter = ROM_START;					// CHIP-8 programs are loaded starting at memory address 0x200
	stackPointer = 0;
	opcode = 0;

	// Initialize memory
	for (int i = 0; i < 4096; i++)
	{
		memory[i] = 0;
	}

	// Initialize stack
	for (int i = 0; i < 16; i++)
	{
		stack[i] = 0;
	}

	// Initialize timers
	delayTimer = 0;
	soundTimer = 0;

	// Store font in memory
	for (int i = 0; i < 80; i++)
	{
		memory[FONT_START + i] = font[i];		// font will be stored in memory from 0x000-0x04F
	}

	// Initialize display
	for (int i = 0; i < DISPLAY_WIDTH * DISPLAY_HEIGHT; i++)
	{
		display[i] = false;
	}

	// Initialize keypad
	for (int i = 0; i < 16; i++)
	{
		keypad[i] = 0;							// 0 represtens NOT PRESSED and 1 represents PRESSED
	}
}

void Chip8::loadROM(const std::string& fileName)
{
	std::ifstream romFile(fileName, std::ios::binary);

	if (!romFile)
	{
		std::cerr << "Failed to open ROM file: " << std::endl;
		return;
	}

	// std::cout << "Rom loaded successfully." << std::endl;

	char byte;
	int memoryIndex = ROM_START;

	while (romFile.get(byte) && memoryIndex < 4096)
	{
		memory[memoryIndex] = static_cast<uint8_t>(byte);
		memoryIndex++;
	}

	romFile.close();
}

void Chip8::cycle()
{
	static auto lastTimerUpdate = std::chrono::steady_clock::now();

	// fetch the next 2-byte instruction from memory
	opcode = fetch();

	// decode the opcode to determine which instruction it represents and execute
	decodeAndExecute();

	// update timers at 60Hz
	auto currentTime = std::chrono::steady_clock::now();

	auto elapsedTime = std::chrono::duration<double>(currentTime - lastTimerUpdate).count();

	if (elapsedTime >= 1.0 / 60.0)
	{
		updateTimers();
		lastTimerUpdate = currentTime;
	}
}

std::vector< sf::RectangleShape> Chip8::printDisplay() const
{
	std::vector<sf::RectangleShape> pixels;

	pixels.reserve(DISPLAY_WIDTH * DISPLAY_HEIGHT);

	for (int y = 0; y < DISPLAY_HEIGHT; y++)
	{
		for (int x = 0; x < DISPLAY_WIDTH; x++)
		{
			if (display[y * DISPLAY_WIDTH + x])
			{
				sf::RectangleShape pixel;
				pixel.setSize({ static_cast<float>(PIXEL_SIZE), static_cast<float>(PIXEL_SIZE) });
				pixel.setPosition({ static_cast<float>(x * PIXEL_SIZE), static_cast<float>(y * PIXEL_SIZE) });
				pixel.setFillColor(sf::Color::White);

				pixels.push_back(pixel);
			}
		}
	}

	return pixels;

}

void Chip8::pressKey(uint8_t key)
{
	if (key < 16)
	{
		keypad[key] = true;
	}
}

void Chip8::releaseKey(uint8_t key)
{
	if (key < 16)
	{
		keypad[key] = false;
	}
}

bool Chip8::isSoundActive() const
{
	return soundTimer > 0;
}

uint16_t Chip8::fetch()
{
	// read instruction from memory
	opcode = (memory[programCounter] << 8) | memory[programCounter + 1];	// combine two consecutive bytes from memory (instructions are 16 bits)

	// move to next instruction
	programCounter += 2;

	return opcode;
}

void Chip8::decodeAndExecute()
{
	// Variables to extract instruction components
	uint8_t X = (opcode & 0x0F00) >> 8;		// second nibble (register selector)
	uint8_t Y = (opcode & 0x00F0) >> 4;		// third nibble (register selector)
	uint8_t N = opcode & 0x000F;			// fourth nibble (4-bit value)
	uint8_t NN = opcode & 0x00FF;			// second byte (8-bit value)
	uint16_t NNN = opcode & 0x0FFF;			// last three nibbles (12-bit address)

	switch (opcode & 0xF000)
	{
	case 0x0000:
		switch (opcode)
		{
		case 0x00E0:	// 00E0 - clear the screen
			for (int i = 0; i < DISPLAY_WIDTH * DISPLAY_HEIGHT; i++)
			{
				display[i] = false;
			}
			break;
		case 0x00EE:	// 00EE - return from subroutine to address pulled from stack
			stackPointer--;
			programCounter = stack[stackPointer];
			break;
		default:		// 0NNN - jump to native assembler subroutine at 0xNNN; not implemented in this emulator
			break;
		}
		break;

	case 0x1000:	// 1NNN - jump to address NNN
		programCounter = NNN;
		break;

	case 0x2000:	// 2NNN - push return address onto stack and call subroutine at address NNN
		stack[stackPointer] = programCounter;
		stackPointer++;
		programCounter = NNN;
		break;

	case 0x3000:	// 3XNN - skip next opcode (instruction) if VX == NN
		if (V[X] == NN)
		{
			programCounter += 2;
		}
		break;

	case 0x4000:	// 4XNN - skip next opcode (instruction) if VX != NN
		if (V[X] != NN)
		{
			programCounter += 2;
		}
		break;

	case 0x5000:	// 5XY0 - skip next opcode (instruction) if VX == VY
		if (V[X] == V[Y])
		{
			programCounter += 2;
		}
		break;

	case 0x6000:	// 6XNN - set VX to NN
		V[X] = NN;
		break;

	case 0x7000:	// 7XNN - add NN to VX
		V[X] += NN;
		break;

	case 0x8000:
		switch (N)
		{
		case 0x0:	// 8XY0 - set VX to the value of VY
			V[X] = V[Y];
			break;
		case 0x1:	// 8XY1 - set VX to the result of bitwise VX OR VY
			V[X] |= V[Y];
			break;
		case 0x2:	// 8XY2 - set VX to the result of bitwise VX AND VY
			V[X] &= V[Y];
			break;
		case 0x3:	// 8XY3 - set VX to the result of bitwise VX XOR VY
			V[X] ^= V[Y];
			break;
		case 0x4:	// 8XY4 - add VY to VX
			V[0xF] = (V[X] + V[Y] > 255);		// V[F] (carry flag) set to 1 if result is greater than 255, otherwise 0
			V[X] += V[Y];
			break;
		case 0x5:	// 8XY5 - subtract VY from VX
			V[0xF] = (V[X] >= V[Y]);			// V[F] (carry flag) set to 1 if V[X] >= V[Y], otherwise 0
			V[X] -= V[Y];
			break;
		case 0x6:	// 8XY6 - set VX to VY and shift VX one bit to the right
			V[X] = V[Y];
			V[0xF] = V[X] & 0x1;				// V[F] (carry flag) set to bit that is shifted out
			V[X] >>= 1;
			break;
		case 0x7:	// 8XY7 - set VX to the result of (VY - VX)
			V[0xF] = (V[X] <= V[Y]);			// V[F] (carry flag) set to 1 if V[X] <= V[Y], otherwise 0
			V[X] = V[Y] - V[X];
			break;
		case 0xE:	// 8XYE - set VX to VY and shift VX one bit to the left
			V[X] = V[Y];
			V[0xF] = (V[X] & 0x80) >> 7;		// V[F] (carry flag) set to bit that is shifted out
			V[X] <<= 1;
			break;
		default:
			break;
		}
		break;

	case 0x9000:	// 9XY0 - skip next opcode (instruction) if VX != VY
		if (V[X] != V[Y])
		{
			programCounter += 2;
		}
		break;

	case 0xA000:	// ANNN - set I to NNN
		I = NNN;
		break;

	case 0xB000:	// BNNN - jump to address NNN + V0
		programCounter = NNN + V[0];
		break;

	case 0xC000:	// CXNN - set VX to a random byte masked with NN (byte AND NN)
		V[X] = (rand() % 256) & NN;
		break;

	case 0xD000:	// DXYN - draw 8xN sprite at position VX and VY with data starting at the address in I (height N)
	{
		uint8_t xPosition = V[X] % DISPLAY_WIDTH;
		uint8_t yPosition = V[Y] % DISPLAY_HEIGHT;

		V[0xF] = 0;		// reset collision flag register

		for (int row = 0; row < N; row++)
		{
			uint8_t spriteByte = memory[I + row];

			for (int column = 0; column < 8; column++)
			{
				uint8_t spritePixel = spriteByte & (0x80 >> column);

				if (spritePixel != 0)
				{
					int x = (xPosition + column) % DISPLAY_WIDTH;
					int y = (yPosition + row) % DISPLAY_HEIGHT;

					int displayIndex = y * DISPLAY_WIDTH + x;

					if (display[displayIndex])
					{
						V[0xF] = 1;		// collision happened
					}

					display[displayIndex] = !display[displayIndex];		// XOR draw
				}
			}
		}
		break;
	}

	case 0xE000:
		switch (NN)
		{
		case 0x9E:	// EX9E - skip next opcode (instruction) if key in the lower 4 bits of VX is pressed
			if (keypad[V[X]])
			{
				programCounter += 2;
			}
			break;
		case 0xA1:	// EXA1 - skip next opcode (instruction) if key in the lower 4 bits of VX is not pressed
			if (!keypad[V[X]])
			{
				programCounter += 2;
			}
			break;
		default:
			break;
		}
		break;

	case 0xF000:
		switch (NN)
		{
		case 0x07:	// FX07 - set VX to value of delay timer
			V[X] = delayTimer;
			break;
		case 0x0A:	// FX0A - wait for a key pressed and released and store that key in VX
		{
			bool keyPressed = false;

			for (int i = 0; i < 16; i++)
			{
				if (keypad[i])
				{
					V[X] = i;
					keyPressed = true;
					break;
				}
			}
			if (!keyPressed)
			{
				programCounter -= 2;
			}
			break;
		}
		case 0x15:	// FX15 - set delay timer to VX
			delayTimer = V[X];
			break;
		case 0x18:	// FX18 - set sound timer to VX
			soundTimer = V[X];
			break;
		case 0x1E:	// FX1E - add VX to I
			I += V[X];
			break;
		case 0x29:	// FX29 - set I to the font sprite for digit VX
			I = V[X] * 5;
			break;
		case 0x33:	// FX33 - store BCD of VX in memory at I, I+1, and I+2
			memory[I] = V[X] / 100;
			memory[I + 1] = (V[X] / 10) % 10;
			memory[I + 2] = V[X] % 10;
			break;
		case 0x55:	// FX55 - store V0 through VX in memory starting at I
			for (int i = 0; i <= X; i++)
			{
				memory[I + i] = V[i];
			}
			break;
		case 0x65:	// FX65 - load V0 through VX from memory starting at I
			for (int i = 0; i <= X; i++)
			{
				V[i] = memory[I + i];
			}
			I += X + 1;
			break;
		default:
			break;
		}
		break;

	default:
		break;
	}
}

void Chip8::updateTimers()
{
	if (delayTimer > 0)
	{
		delayTimer--;
	}

	if (soundTimer > 0)
	{
		soundTimer--;
	}
}