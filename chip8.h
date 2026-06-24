#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

typedef struct CHIP8{
    uint8_t memory[4096];

    //V0 to VF
    uint8_t registers[16];
    
    //address register
    uint16_t I;

    //program counter
    uint16_t pc;

    //Stack
    uint16_t stack[16];
    uint8_t sp;

    //Timers
    uint8_t delay_timer;
    uint8_t sound_timer;

    //Input
    uint8_t key; 

    //Graphics and Sound
    bool graphics[64*32];

    //Opcode
    uint16_t opcode;
}CHIP8;


struct CHIP8* initCHIP8();
void fetch(struct CHIP8* chip);
uint16_t decodeInstruction(uint16_t instruction, uint16_t mask, uint16_t deslocate);
void decode(struct CHIP8* chip);
