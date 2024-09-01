//
// Created on 2023/11/22.
//

#include "stdio.h"
#include "_bsd_types.h"

#include "game.h"


void printPlayerInput(int correctCount, u_char* string){
    printf("player:\n");
    for(int i = 0; i < correctCount; i++){
        printf("%c", string[i]);
    }
}

