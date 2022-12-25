#include <cstdint>
#include <random>

class CPU {
    public:
        // Constructor
        CPU();

        // Functions
        void LoadROM(char const* filename);
        void Cycle();

        // Opcodes (34 in total + opNull)
        void op00E0();
        void op00EE();
        void op1nnn();
        void op2nnn();
        void op3xkk();
        void op4xkk();
        void op5xy0();
        void op6xkk();
        void op7xkk();
        void op8xy0();
        void op8xy1();
        void op8xy2();
        void op8xy3();
        void op8xy4();
        void op8xy5();
        void op8xy6();
        void op8xy7();
        void op8xyE();
        void op9xy0();
        void opAnnn();
        void opBnnn();
        void opCxkk();
        void opDxyn();
        void opEx9E();
        void opExA1();
        void opFx07();
        void opFx0A();
        void opFx15();
        void opFx18();
        void opFx1E();
        void opFx29();
        void opFx33();
        void opFx55();
        void opFx65();
        void opNull();

        // Table Function for opcode
        void Table0();
	    void Table8();
	    void TableE();
	    void TableF();

        typedef void (CPU::*CPUFunc)();
	    CPUFunc table[0xF + 1];
	    CPUFunc table0[0xE + 1];
	    CPUFunc table8[0xE + 1];
	    CPUFunc tableE[0xE + 1];
	    CPUFunc tableF[0x65 + 1];

        // Variables
        uint8_t registers[16]{};
        uint8_t memory[4096]{};
        uint16_t indexRegister{};
        uint16_t programCounter{};
        uint16_t stack[16]{};
        uint8_t stackPointer{};
        uint8_t delayTimer{};
        uint8_t soundTimer{};
        uint8_t keys[16]{};
        uint32_t display[64 * 32]{};
        uint16_t opcode{};
        
        // Random Number Generator
        std::default_random_engine randGen;
	    std::uniform_int_distribution<uint8_t> randByte;
};