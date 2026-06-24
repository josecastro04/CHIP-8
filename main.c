#include <stdio.h>
#include "chip8.h"
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

int main(int argc, char **argv){
    if(argc != 2){
        printf("./main <Chip-8 ROM>");
        return -1;
    }
    printf("%s %s\n", argv[0], argv[1]);
    
    struct CHIP8* chip = initCHIP8();

    int fd = open(argv[1], O_RDONLY);
    if(fd == -1){
        printf("Error while opening the file!");
        return -1;
    }
    uint16_t start_pos = 0x200;
    uint8_t buffer[256];
    ssize_t bytes = 0;
    while((bytes = read(fd, buffer, 256)) > 0){
        //printf("%ld\n", bytes);
        for(ssize_t i = 0; i < bytes; i++){
            chip->memory[start_pos + i] = buffer[i];
            //printf("%x", chip->memory[start_pos + i]);
        }
        start_pos += bytes;
    }

    close(fd);
    for(;;){
        fetch(chip);
        decode(chip);
        printf("\033[H");
        for(uint8_t y = 0; y < 32; y++){
            for(uint8_t x = 0; x < 64; x++){
                char pixel = chip->graphics[x + y * 64] ? '#' : ' ';
                printf("%c", pixel);
            }
            printf("\n");
        }
        fflush(stdout);
    }
   /* 
    printf("\n\n");
    for(int i = 0x200; i < 0x200 + 148; i++){
        printf("%x", chip->memory[i]);
    }

    int x;
    scanf("%d", &x);
    for(int y = 0; y < 32; y++){
        for(int x = 0; x < 64; x++){
            printf("%d", chip->graphics[x + y * 64]);
        }
        printf("\n");
    }*/
    return 0;
}
