#include "cpu.h"
#include <fstream>
#include <cstdint>
#include <chrono>
#include <random>
#include <cstring>

// Starting address
const unsigned int startAddress = 0x200;

// 16 characters with 5 bytes each
const unsigned int fontSize = 16 * 5;

// Starting Address for font
const unsigned int fontStartAddress = 0x50;

// Array of bytes to load onto memory
uint8_t font[fontSize] = {
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

// Constructor for the CPU class
CPU::CPU() 
    // uses system time as seed
    : randGen(std::chrono::system_clock::now().time_since_epoch().count()) {

    // Set the pc to the starting address
    programCounter = startAddress;

    // Loads font into memory
    for (unsigned int i = 0; i < fontSize; ++i) {
        memory[fontStartAddress + i] = font[i];
    }

    // Initializes the random number generator between 0 and 255
    randByte = std::uniform_int_distribution<uint8_t>(0, 255U);

    // A function pointer table for the opcodes
    table[0x0] = &CPU::Table0;
	table[0x1] = &CPU::op1nnn;
	table[0x2] = &CPU::op2nnn;
	table[0x3] = &CPU::op3xkk;
	table[0x4] = &CPU::op4xkk;
	table[0x5] = &CPU::op5xy0;
	table[0x6] = &CPU::op6xkk;
	table[0x7] = &CPU::op7xkk;
	table[0x8] = &CPU::Table8;
	table[0x9] = &CPU::op9xy0;
	table[0xA] = &CPU::opAnnn;
	table[0xB] = &CPU::opBnnn;
	table[0xC] = &CPU::opCxkk;
	table[0xD] = &CPU::opDxyn;
	table[0xE] = &CPU::TableE;
	table[0xF] = &CPU::TableF;

	for (size_t i = 0; i <= 0xE; i++)
	{
		table0[i] = &CPU::opNull;
		table8[i] = &CPU::opNull;
		tableE[i] = &CPU::opNull;
	}

	table0[0x0] = &CPU::op00E0;
	table0[0xE] = &CPU::op00EE;
	table8[0x0] = &CPU::op8xy0;
	table8[0x1] = &CPU::op8xy1;
	table8[0x2] = &CPU::op8xy2;
	table8[0x3] = &CPU::op8xy3;
	table8[0x4] = &CPU::op8xy4;
	table8[0x5] = &CPU::op8xy5;
	table8[0x6] = &CPU::op8xy6;
	table8[0x7] = &CPU::op8xy7;
	table8[0xE] = &CPU::op8xyE;
	tableE[0x1] = &CPU::opExA1;
	tableE[0xE] = &CPU::opEx9E;

	for (size_t i = 0; i <= 0x65; i++) {
		tableF[i] = &CPU::opNull;
	}

	tableF[0x07] = &CPU::opFx07;
	tableF[0x0A] = &CPU::opFx0A;
	tableF[0x15] = &CPU::opFx15;
	tableF[0x18] = &CPU::opFx18;
	tableF[0x1E] = &CPU::opFx1E;
	tableF[0x29] = &CPU::opFx29;
	tableF[0x33] = &CPU::opFx33;
	tableF[0x55] = &CPU::opFx55;
	tableF[0x65] = &CPU::opFx65;
}

// Function Pointer Table
void CPU::Table0() {
	((*this).*(table0[opcode & 0x000Fu]))();
}
void CPU::Table8() {
	((*this).*(table8[opcode & 0x000Fu]))();
}
void CPU::TableE() {
	((*this).*(tableE[opcode & 0x000Fu]))();
}
void CPU::TableF() {
	((*this).*(tableF[opcode & 0x00FFu]))();
}

// Function that loads the ROM onto the CPU
void CPU::LoadROM(char const* filename) {
    // Opens the file as a stream of binary data before moving the file pointer to the end
    std::ifstream file(filename, std::ios::binary | std::ios::ate);

    if (file.is_open()) {

        // Reads the size of the file and initializes a buffer to store its contents
        std::streampos size = file.tellg();
        char* buffer = new char[size];

        // Moves pointer to the beginning to store the content in the buffer
        file.seekg(0, std::ios::beg);
		file.read(buffer, size);
		file.close();

        // Load the content into the CPU's memory via the buffer
        // NOTE: memory is only updated from 0x200 onwards
        for (long i = 0; i < size; ++i)
		{
			memory[startAddress + i] = buffer[i];
        }

        // Since the buffer is no longer used, it is deleted
        delete[] buffer;
    }
}

// Function that starts the fetch, decode, and execute cycle for the CPU
void CPU::Cycle() {
    // Fetch
	opcode = (memory[programCounter] << 8u) | memory[programCounter + 1];

	// Increment the PC before we execute anything
	programCounter += 2;

	// Decode and Execute
	((*this).*(table[(opcode & 0xF000u) >> 12u]))();

	// Decrement the delay timer if it's been set
	if (delayTimer > 0) {
		--delayTimer;
	}

	// Decrement the sound timer if it's been set
	if (soundTimer > 0) {
		--soundTimer;
	}
}

/* ---------- Opcodes ---------- */

// Dummmy function that does nothing (set as default if no other function has been set)
void CPU::opNull() {}

// CLS: clear the display
void CPU::op00E0() {
    memset(display, 0, sizeof(display));
}

// RET: return
void CPU::op00EE() {
    --stackPointer;
    programCounter = stack[stackPointer];
}

// JP addr: jump to nnn
void CPU::op1nnn() {
    uint16_t address = opcode & 0x0FFFu;
    programCounter = address;
}

// CALL addr: call nnn
void CPU::op2nnn() {
    uint16_t address = opcode & 0x0FFFu;
    stack[stackPointer] = programCounter;
    ++stackPointer;
    programCounter = address;
}

// SE Vx, byte: Skip next instruction if Vx = kk
void CPU::op3xkk() {
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t byte = opcode & 0x00FFu;

	if (registers[Vx] == byte) {
		programCounter += 2;
	}
}

// SNE Vx, byte: Skip next instruction if Vx != kk
void CPU::op4xkk() {
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t byte = opcode & 0x00FFu;

	if (registers[Vx] != byte) {
		programCounter += 2;
	}
}

// SE Vx, Vy: Skip next instruction if Vx = Vy
void CPU::op5xy0() {
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t Vy = (opcode & 0x00F0u) >> 4u;

	if (registers[Vx] == registers[Vy]) {
		programCounter += 2;
	}
}

// LD Vx, byte: Set Vx = kk
void CPU::op6xkk() {
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t byte = opcode & 0x00FFu;

	registers[Vx] = byte;
}

// ADD Vx, byte: Set Vx = Vx + kk
void CPU::op7xkk() {
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t byte = opcode & 0x00FFu;

	registers[Vx] += byte;
}

// LD Vx, Vy: Set Vx = Vy
void CPU::op8xy0() {
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t Vy = (opcode & 0x00F0u) >> 4u;

	registers[Vx] = registers[Vy];
}

// OR Vx, Vy: Set Vx = Vx OR Vy
void CPU::op8xy1() {
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t Vy = (opcode & 0x00F0u) >> 4u;

	registers[Vx] |= registers[Vy];
}

// AND Vx, Vy: Set Vx = Vx AND Vy
void CPU::op8xy2() {
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t Vy = (opcode & 0x00F0u) >> 4u;

	registers[Vx] &= registers[Vy];
}

// XOR Vx, Vy: Set Vx = Vx XOR Vy
void CPU::op8xy3() {
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t Vy = (opcode & 0x00F0u) >> 4u;

	registers[Vx] ^= registers[Vy];
}

// ADD Vx, Vy: Set Vx = Vx + Vy, set VF = carry
void CPU::op8xy4() {
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t Vy = (opcode & 0x00F0u) >> 4u;

	uint16_t sum = registers[Vx] + registers[Vy];

	if (sum > 255U) {
		registers[0xF] = 1;
	} else {
		registers[0xF] = 0;
	}

	registers[Vx] = sum & 0xFFu;
}

// SUB Vx, Vy: Set Vx = Vy, set VF = NOT borrow
void CPU::op8xy5() {
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t Vy = (opcode & 0x00F0u) >> 4u;

	if (registers[Vx] > registers[Vy]) {
		registers[0xF] = 1;
	} else {
		registers[0xF] = 0;
	}

	registers[Vx] -= registers[Vy];
}

// SHR Vx: Set Vx = Vx SHR 1
void CPU::op8xy6() {
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;

	// Save least significant bit in VF
	registers[0xF] = (registers[Vx] & 0x1u);

	registers[Vx] >>= 1;
}

// SUBN Vx, Vy: Set Vx = Vy - Vx, set VF = NOT borrow
void CPU::op8xy7() {
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t Vy = (opcode & 0x00F0u) >> 4u;

	if (registers[Vy] > registers[Vx]) {
		registers[0xF] = 1;
	} else {
		registers[0xF] = 0;
	}

	registers[Vx] = registers[Vy] - registers[Vx];
}

// SHL Vx: Set Vx = Vx SHL 1
void CPU::op8xyE() {
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;

	// Save most significant bit in VF
	registers[0xF] = (registers[Vx] & 0x80u) >> 7u;

	registers[Vx] <<= 1;
}

// SNE Vx, Vy: Skip next instruction if Vx != Vy
void CPU::op9xy0() {
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t Vy = (opcode & 0x00F0u) >> 4u;

	if (registers[Vx] != registers[Vy]) {
		programCounter += 2;
	}
}

// LD I, addr: Set I = nnn
void CPU::opAnnn() {
    uint16_t address = opcode & 0x0FFFu;

	indexRegister = address;
}

// JP V0, addr: Jump to nnn + V0
void CPU::opBnnn() {
    uint16_t address = opcode & 0x0FFFu;

	programCounter = registers[0] + address;
}

// RND Vx, byte: Set Vx = random byte AND kk
void CPU::opCxkk() {
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t byte = opcode & 0x00FFu;

	registers[Vx] = randByte(randGen) & byte;
}

// DRW Vx, Vy, nibble: Display n-byte sprite at memory location I at (Vx, Vy) set VF = collision
void CPU::opDxyn() {
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t Vy = (opcode & 0x00F0u) >> 4u;
	uint8_t height = opcode & 0x000Fu;

	// Wrap if going beyond screen boundaries
	uint8_t xPos = registers[Vx] % 64;
	uint8_t yPos = registers[Vy] % 32;

	registers[0xF] = 0;

	for (unsigned int row = 0; row < height; ++row) {
		uint8_t spriteByte = memory[indexRegister + row];
		for (unsigned int col = 0; col < 8; ++col) {
			uint8_t spritePixel = spriteByte & (0x80u >> col);
			uint32_t* screenPixel = &display[(yPos + row) * 64 + (xPos + col)];
			// Sprite pixel is on
			if (spritePixel) {
				// Screen pixel also on - collision
				if (*screenPixel == 0xFFFFFFFF) {
					registers[0xF] = 1;
				}
				// Effectively XOR with the sprite pixel
				*screenPixel ^= 0xFFFFFFFF;
			}
		}
	}
}

// SKP Vx: Skip next instruction if key with value Vx is entered
void CPU::opEx9E() {
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t key = registers[Vx];

	if (keys[key]) {
		programCounter += 2;
	}
}

// SKNP Vx: Skip next instruction if key with value Vx is NOT entered
void CPU::opExA1() {
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t key = registers[Vx];

	if (!keys[key]) {
		programCounter += 2;
	}
}

// LD Vx, DT: Set Vx = delay timer value
void CPU::opFx07() {
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;

	registers[Vx] = delayTimer;
}

// LD Vx, K: Wait for a key press and stores that value in Vx
void CPU::opFx0A() {
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    
    if (keys[0]) {
		registers[Vx] = 0;
	} else if (keys[1]) {
		registers[Vx] = 1;
	} else if (keys[2]) {
		registers[Vx] = 2;
	} else if (keys[3]) {
		registers[Vx] = 3;
	} else if (keys[4]) {
		registers[Vx] = 4;
	} else if (keys[5]) {
		registers[Vx] = 5;
	} else if (keys[6]) {
		registers[Vx] = 6;
	} else if (keys[7]) {
		registers[Vx] = 7;
	} else if (keys[8]) {
		registers[Vx] = 8;
	} else if (keys[9]) {
		registers[Vx] = 9;
	} else if (keys[10]) {
		registers[Vx] = 10;
	} else if (keys[11]) {
		registers[Vx] = 11;
	} else if (keys[12]) {
		registers[Vx] = 12;
	} else if (keys[13]) {
		registers[Vx] = 13;
	} else if (keys[14]) {
		registers[Vx] = 14;
	} else if (keys[15]) {
		registers[Vx] = 15;
	} else {
		programCounter -= 2;
	}
}

// LD DT, Vx: Set delay timer = Vx
void CPU::opFx15() {
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;

	delayTimer = registers[Vx];
}

// LD ST, Vx: Set sound timer = Vx
void CPU::opFx18() {
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;

	soundTimer = registers[Vx];
}

// ADD I, Vx: Set I = I + Vx
void CPU::opFx1E() {
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;

	indexRegister += registers[Vx];
}

// LD F, Vx: Set I = location of sprite for digit Vx
void CPU::opFx29() {
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t digit = registers[Vx];

	indexRegister = fontStartAddress + (5 * digit);
}

// LD B, Vx: Store binary coded decimal of Vx in memory locations I, I+1, and I+2
void CPU::opFx33() {
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t value = registers[Vx];

	// Ones-place
	memory[indexRegister + 2] = value % 10;
	value /= 10;

	// Tens-place
	memory[indexRegister + 1] = value % 10;
	value /= 10;

	// Hundreds-place
	memory[indexRegister] = value % 10;
}

// LD [I], Vx: Store registers V0 to Vx in memory starting from location I
void CPU::opFx55() {
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;

	for (uint8_t i = 0; i <= Vx; ++i) {
		memory[indexRegister + i] = registers[i];
	}
}

// LD Vx, [I]: Read registers V0 to Vx from memory starting from location I
void CPU::opFx65() {
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;

	for (uint8_t i = 0; i <= Vx; ++i) {
		registers[i] = memory[indexRegister + i];
	}
}