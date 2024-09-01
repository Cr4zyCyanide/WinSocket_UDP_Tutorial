//
// Created on 2023/11/21.
//

#ifndef TYPING_BATTLE_S_FILES_H
#define TYPING_BATTLE_S_FILES_H

#include "game.h"

int writeUserInfo_IntoFile(char *username, char *passwd);
int readUserInfo_FromFile(char *username, char *passwd, int flag);
ptr_rksHead readRksInfo_FromFile(ptr_rksHead head);
int writeNewRecord_IntoFile(ptr_node newNode);
void logPrintf(int flag, const char *format, ...);

#endif //TYPING_BATTLE_S_FILES_H
