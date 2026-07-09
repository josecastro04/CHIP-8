#include <SDL2/SDL.h>
#include <stdio.h>
#include "chip8.h"
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

#define SCREEN_WIDTH 960
#define SCREEN_HEIGHT 320
#define WINDOW_TITLE "Chip-8"


struct Screen{
    SDL_Window *window;
    SDL_Renderer *renderer;
};

int sdl_init(struct Screen *screen){

    if(SDL_Init(SDL_INIT_VIDEO)){
        fprintf(stderr, "Error initializing SDL: %s\n", SDL_GetError());
        return 1;
    }
    
    screen->window = SDL_CreateWindow(WINDOW_TITLE, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, 0);
    if(!screen->window){
        fprintf(stderr, "Error creating window: %s\n", SDL_GetError());
        return 1;
    }

    screen->renderer = SDL_CreateRenderer(screen->window, -1, 0);
    if(!screen->renderer){
        fprintf(stderr, "Error creating render: %s\n", SDL_GetError());
        return 1;
    }
    return 0;
}

void screen_cleanup(struct Screen *screen, int exit_status){
    SDL_DestroyRenderer(screen->renderer);
    SDL_DestroyWindow(screen->window);
    SDL_Quit();

    exit(exit_status);
}

void load_ROM(struct CHIP8 *chip, char *filename){
    int fd = open(filename, O_RDONLY);
    if(fd == -1){
        perror("Error while opening the file!");
        exit(EXIT_FAILURE);
    }

    uint16_t start_pos = 0x200;
    uint8_t buffer[256];
    ssize_t bytes = 0;
    while((bytes = read(fd, buffer, 256)) > 0){
        for(ssize_t i = 0; i < bytes; i++){
            chip->memory[start_pos++] = buffer[i];
        }
    }

    close(fd);
}

void drawScreen(struct Screen *screen, struct CHIP8 *chip){
    SDL_Rect rects[64 * 32];
    int number_of_rects = 0;
    for(uint8_t y = 0; y < 32; y++){
        for(uint8_t x = 0; x < 64; x++){
            if(chip->graphics[x + y * 64])
                rects[number_of_rects++] = (SDL_Rect){ .x = x * 15, .y = y * 10, .w = 14, .h = 9};
        }
    }

    SDL_SetRenderDrawColor(screen->renderer, 255, 255, 255, 0);
    if(SDL_RenderFillRects(screen->renderer, rects, number_of_rects)){
        fprintf(stderr, "Error drawing rects: %s\n", SDL_GetError());
        screen_cleanup(screen, EXIT_FAILURE);
    }
    SDL_SetRenderDrawColor(screen->renderer, 0, 0, 0, 0);
}


int main(int argc, char **argv){
    
    if(argc != 2){
        perror("./main <Chip-8 ROM>");
        exit(EXIT_FAILURE);
    }

    struct Screen screen = {
        .window = NULL,
        .renderer = NULL,
    };

    if(sdl_init(&screen)){
        screen_cleanup(&screen, EXIT_FAILURE);
    }

    struct CHIP8* chip = initCHIP8();

    load_ROM(chip, argv[1]);

    uint8_t instruction_counter;
    uint64_t freq = SDL_GetPerformanceFrequency();
    while(1){
        SDL_Event event;
        while(SDL_PollEvent(&event)){
            switch(event.type){
                case SDL_QUIT:
                    screen_cleanup(&screen, EXIT_SUCCESS);
                    break;
                case SDL_KEYDOWN:
                    switch(event.key.keysym.scancode){
                        case SDL_SCANCODE_ESCAPE:
                            screen_cleanup(&screen, EXIT_SUCCESS);
                            break;
                        case SDL_SCANCODE_1:
                            chip->keys[0x1] = 1;
                            break;
                        case SDL_SCANCODE_2:
                            chip->keys[0x2] = 1;
                            break;
                        case SDL_SCANCODE_3:
                            chip->keys[0x3] = 1;
                            break;
                        case SDL_SCANCODE_4:
                            chip->keys[0xC] = 1;
                            break;
                        case SDL_SCANCODE_Q:
                            chip->keys[0x4] = 1;
                            break;
                        case SDL_SCANCODE_W:
                            chip->keys[0x5] = 1;
                            break;
                        case SDL_SCANCODE_E:
                            chip->keys[0x6] = 1;
                            break;
                        case SDL_SCANCODE_R:
                            chip->keys[0xD] = 1;
                            break;
                        case SDL_SCANCODE_A:
                            chip->keys[0x7] = 1;
                            break;
                        case SDL_SCANCODE_S:
                            chip->keys[0x8] = 1;
                            break;
                        case SDL_SCANCODE_D:
                            chip->keys[0x9] = 1;
                            break;
                        case SDL_SCANCODE_F:
                            chip->keys[0xE] = 1;
                            break;
                        case SDL_SCANCODE_Z:
                            chip->keys[0xA] = 1;
                            break;
                        case SDL_SCANCODE_X:
                            chip->keys[0x0] = 1;
                            break;
                        case SDL_SCANCODE_C:
                            chip->keys[0xB] = 1;
                            break;
                        case SDL_SCANCODE_V:
                            chip->keys[0xF] = 1;
                            break;

                        default:
                            break;
                    }
                        break;
                    case SDL_KEYUP:
                    switch(event.key.keysym.scancode){
                        case SDL_SCANCODE_ESCAPE:
                            screen_cleanup(&screen, EXIT_SUCCESS);
                            break;
                        case SDL_SCANCODE_1:
                            chip->keys[0x1] = 0;
                            break;
                        case SDL_SCANCODE_2:
                            chip->keys[0x2] = 0;
                            break;
                        case SDL_SCANCODE_3:
                            chip->keys[0x3] = 0;
                            break;
                        case SDL_SCANCODE_4:
                            chip->keys[0xC] = 0;
                            break;
                        case SDL_SCANCODE_Q:
                            chip->keys[0x4] = 0;
                            break;
                        case SDL_SCANCODE_W:
                            chip->keys[0x5] = 0;
                            break;
                        case SDL_SCANCODE_E:
                            chip->keys[0x6] = 0;
                            break;
                        case SDL_SCANCODE_R:
                            chip->keys[0xD] = 0;
                            break;
                        case SDL_SCANCODE_A:
                            chip->keys[0x7] = 0;
                            break;
                        case SDL_SCANCODE_S:
                            chip->keys[0x8] = 0;
                            break;
                        case SDL_SCANCODE_D:
                            chip->keys[0x9] = 0;
                            break;
                        case SDL_SCANCODE_F:
                            chip->keys[0xE] = 0;
                            break;
                        case SDL_SCANCODE_Z:
                            chip->keys[0xA] = 0;
                            break;
                        case SDL_SCANCODE_X:
                            chip->keys[0x0] = 0;
                            break;
                        case SDL_SCANCODE_C:
                            chip->keys[0xB] = 0;
                            break;
                        case SDL_SCANCODE_V:
                            chip->keys[0xF] = 0;
                            break;
                        default:
                            break;
                        }
                default:
                    break;
            }
        }
        uint64_t initial = SDL_GetPerformanceCounter();
        instruction_counter = 1;
        while(instruction_counter <= 10){
            fetch(chip);
            decode(chip);
            instruction_counter++;
        }
        
        SDL_RenderClear(screen.renderer);
        drawScreen(&screen, chip);
        SDL_RenderPresent(screen.renderer);
       
        if(chip->delay_timer > 0) chip->delay_timer--;

        if(chip->sound_timer > 0) chip->sound_timer--;

        uint64_t finish = SDL_GetPerformanceCounter();
        float elapsed = (float)(finish - initial) * 1000 / freq;
        float target = 1000.f / 60;
        
        if(elapsed < target){
            SDL_Delay(target - elapsed);
        }
    }

    screen_cleanup(&screen, EXIT_SUCCESS);
    return 0;
}
