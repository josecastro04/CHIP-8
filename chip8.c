#include "chip8.h"
#include <stdio.h>

void setFont(struct CHIP8 *chip){
    //font
    uint8_t values[] = 
    { 0xF0, 0x90, 0x90, 0x90, 0xF0, //0
      0x20, 0x60, 0x20, 0x20, 0x70, //1
      0xF0, 0x10, 0xF0, 0x80, 0xF0, //2
      0xF0, 0x10, 0XF0, 0x10, 0xF0, //3
      0x90, 0x90, 0xF0, 0x10, 0x10, //4
      0xF0, 0x80, 0xF0, 0x10, 0xF0, //5
      0xF0, 0x80, 0xF0, 0X90, 0xF0, //6
      0xF0, 0x10, 0x20, 0x40, 0x40, //7
      0xF0, 0x90, 0xF0, 0x90, 0xF0, //8
      0xF0, 0x90, 0xF0, 0x10, 0xF0, //9
      0xF0, 0x90, 0xF0, 0x90, 0x90, //A
      0xE0, 0x90, 0xE0, 0x90, 0xE0, //B
      0xF0, 0x80, 0x80, 0x80, 0xF0, //C
      0xE0, 0x90, 0x90, 0x90, 0xE0, //D
      0xF0, 0x80, 0xF0, 0x80, 0xF0, //E
      0xF0, 0x80, 0xF0, 0x80, 0x80  //F
    };

    for(uint8_t i = 0; i < 80; i++){
        chip->memory[i + 0x50] = values[i]; 
    }
}

struct CHIP8* initCHIP8(){
    struct CHIP8* chip = malloc(sizeof(CHIP8));
    if(chip == NULL) exit(0); 
    memset(chip->memory, 0, 4096 * sizeof(uint8_t));
    memset(chip->registers, 0, 16 * sizeof(uint8_t));
    chip->I = 0x0000;
    chip->pc = 0x200;
    chip->sp = 0x0000;
    chip->delay_timer = 0x00;
    chip->sound_timer = 0x00;
    memset(chip->graphics, 0, 64 * 32 * sizeof(bool));
    chip->opcode = 0x0000;
    memset(chip->keys, 0, 16 * sizeof(bool));
    setFont(chip);

    return chip;
}

void fetch(struct CHIP8* chip){
    uint16_t instruction = (chip->memory[chip->pc] << 8) | chip->memory[chip->pc + 1];
    chip->pc += 2;
    chip->opcode = instruction;
}

uint16_t decodeInstruction(uint16_t instruction, uint16_t mask, uint16_t deslocate){
    return (instruction & mask) >> deslocate;
}

void decode(struct CHIP8* chip){
    uint16_t instruction = chip->opcode;
    uint8_t X = decodeInstruction(instruction, 0x0F00, 8);
    uint8_t Y = decodeInstruction(instruction, 0x00F0, 4);
    uint8_t N = decodeInstruction(instruction, 0x000F, 0);
    uint8_t NN = decodeInstruction(instruction, 0x00FF, 0);
    uint16_t NNN = decodeInstruction(instruction, 0x0FFF, 0);

    switch((instruction & 0xF000) >> 12){
        case 0x0:
            switch(NNN){
                case 0x0E0:
                        memset(chip->graphics, 0, 64 * 32 * sizeof(bool));
                    break;
                case 0x0EE:
                    chip->sp--;
                    chip->pc = chip->stack[chip->sp];
                    break;
            }
            break;
        case 0x1:
            chip->pc = NNN;
            break;
        case 0x2:
            chip->stack[chip->sp] = chip->pc;
            chip->sp++;
            chip->pc = NNN;
            break;
        case 0x3:
            chip->pc += 2 * (chip->registers[X] == NN);
            break;
        case 0x4:
            chip->pc += 2 * (chip->registers[X] != NN);
            break;
        case 0x5:
            chip->pc += 2 * (chip->registers[X] == chip->registers[Y]);
            break;
        case 0x6:
            chip->registers[X] = NN;
            break;
        case 0x7:
            chip->registers[X] += NN;
            break;
        case 0x8:
                switch(N){
                    case 0:
                        chip->registers[X] = chip->registers[Y];
                        break;
                    case 1:
                        chip->registers[X] |= chip->registers[Y];
                        chip->registers[0xF] = 0x0; //NOTE: CHIP-8 only
                        break;
                    case 2:
                        chip->registers[X] &= chip->registers[Y];
                        chip->registers[0xF] = 0x0; //NOTE: CHIP-8 only
                        break;
                    case 3:
                        chip->registers[X] ^= chip->registers[Y];
                        chip->registers[0xF] = 0x0; //NOTE: CHIP-8 only
                        break;
                    case 4:
                    {
                        uint16_t result = chip->registers[X] + chip->registers[Y];
                        chip->registers[X] = (uint8_t)result;
                        chip->registers[0xF] = result > 0xFF;
                        break;
                    }
                    case 5:
                    {
                        uint8_t register_X = chip->registers[X];
                        uint16_t result = register_X - chip->registers[Y];
                        chip->registers[X] = (uint8_t)result;
                        chip->registers[0xF] = register_X >= chip->registers[Y];
                        break;
                    }
                    case 6:
                    {
                        uint8_t register_Y = chip->registers[Y];
                        chip->registers[X] = register_Y >> 1;
                        chip->registers[0xF] = register_Y & 0x1;
                        break;
                    }
                    case 7:
                    {
                        uint8_t register_X = chip->registers[X];
                        uint16_t result = chip->registers[Y] - register_X;
                        chip->registers[X] = (uint8_t)result;
                        chip->registers[0xF] = chip->registers[Y] >= register_X;
                        break;
                    }
                    case 0xE:
                    {
                        uint8_t register_Y = chip->registers[Y];
                        chip->registers[X] = register_Y << 0x1;
                        chip->registers[0xF] = (register_Y & 0x80) >> 0x7;
                        break;
                    }
                }
            break;
        case 0x9:
            chip->pc += (0x2 * (chip->registers[X] != chip->registers[Y]));
            break;
        case 0xA:
            chip->I = NNN;
            break;
        case 0xB:
            chip->pc = NNN + chip->registers[0x0];
            break;
        case 0xC:
                chip->registers[X] = (rand() % 0x100) & NN;
            break;
        case 0xD:
            chip->registers[0xF] = 0x0;
            uint8_t coordinate_X = chip->registers[X] % 0x40;
            uint8_t coordinate_Y = chip->registers[Y] % 0x20;
            for(uint8_t i = 0x0; i < N; ++i){
                uint8_t sprite = chip->memory[chip->I + i];
               
                for(uint8_t j = 0x0; j < 0x8; j++){
                    bool sprite_bit = (sprite >> (0x8 - (j + 0x1))) & 0x1;
                    bool graphics_bit = chip->graphics[(coordinate_X + j) + (coordinate_Y + i)  * 0x40];
                    chip->registers[0xF] |= (sprite_bit & graphics_bit);
                    chip->graphics[(coordinate_X + j) + (coordinate_Y + i) * 0x40] ^= sprite_bit;
                    
                    if(coordinate_X + j == 0x3F) break;
                }

                if(coordinate_Y + i == 0x1F) break;

            }
            break;
        case 0xE:
            switch(NN){
                case 0x9E:
                    chip->pc += 0x2 * (chip->keys[chip->registers[X]] == 0x1);
                    break;
                case 0xA1:
                    chip->pc += 0x2 * (chip->keys[chip->registers[X]] != 0x1);
                    break;
            }
            break;
        case 0xF:
            switch(instruction & 0x00FF){
                case 0x07:
                    chip->registers[X] = chip->delay_timer;
                    break;
                case 0x0A:
                    {
                        static bool pressed = 0x0;
                        static uint8_t key = 0xFF;
                        for(uint8_t i = 0x0; key == 0xFF && i < 0x10; i++){
                            if(chip->keys[i] == 0x1){
                                key = i;
                                pressed = 0x1;
                                break;
                            }
                        }
                        if(!pressed) chip->pc -= 0x2;
                        else{
                            if(chip->keys[key]) chip->pc -= 0x2;
                            else{
                                chip->registers[X] = key;
                                key = 0xFF;
                                pressed = 0x0;
                            }
                        }
                            
                        break;
                    }
                case 0x15:
                    chip->delay_timer = chip->registers[X];
                    break;
                case 0x18:
                    chip->sound_timer = chip->registers[X];
                    break;
                case 0x1E:
                    chip->I += chip->registers[X];
                    break;
                case 0x29:
                    chip->I = (chip->registers[X] & 0x0F) * 5 + 0x50;
                    break;
                case 0x33:{
                    uint8_t value = chip->registers[X];
                    chip->memory[chip->I + 2] = value % 10;
                    value /= 10;
                    chip->memory[chip->I + 1] = value % 10;
                    value /= 10;
                    chip->memory[chip->I] = value;
                    break;
                          }
                case 0x55:
                    for(uint8_t i = 0x0; i <= X; i++){
                        chip->memory[chip->I + i] = chip->registers[i];
                    }
                    chip->I += X + 1;
                    break;
                case 0x65:
                    for(uint8_t i = 0x0; i <= X; i++){
                        chip->registers[i] = chip->memory[chip->I + i];
                    }
                    chip->I += X + 1;
                    break;
            }
            break;
    }
}
