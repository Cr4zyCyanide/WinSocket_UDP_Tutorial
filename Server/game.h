//
// Created on 2023/11/21.
//

#ifndef TYPING_BATTLE_S_GAME_H
#define TYPING_BATTLE_S_GAME_H

#include "stdio.h"
#include "malloc.h"
#include "time.h"
#include "string.h"

#define MAX_STRING_LENGTH 8

#define bool unsigned char
#define false 0
#define true 1

struct PLAYERS{
    char no[5];
    char username[64];
    char timeUsed[16];
    char avgSpeed[16];
    char acc[16];
    char score[16];
    struct PLAYERS *next;
};

typedef struct PLAYERS playerData;
typedef struct PLAYERS *ptr_playerData;
typedef struct PLAYERS *ptr_rksHead;
typedef ptr_rksHead ptr_node;


ptr_rksHead rksList_Refresh(ptr_rksHead head);
void insertByScore(ptr_rksHead head, ptr_node newNode);
void rksPrint(ptr_rksHead head);
void updateRanking(ptr_rksHead head, ptr_playerData newNode);
void printAndNumberList(ptr_node head);
bool usernameExists(ptr_rksHead head, const char *username);
void updateHighestScore(ptr_rksHead head, ptr_node newNode);
char* getRksAsString(ptr_rksHead head);
void freeList(ptr_node head);

ptr_node getTargetInfo(ptr_rksHead head, char* no);
void getChallengeResult(ptr_playerData data, char* buffer);

char *randString(int length);
double scoreCalculator(double acc, double timeUsed);


#endif //TYPING_BATTLE_S_GAME_H
