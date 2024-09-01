//
// Created on 2023/11/22.
//

#ifndef TYPING_BATTLE_C_NET_H
#define TYPING_BATTLE_C_NET_H

#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "winsock2.h"
#include "ws2tcpip.h"
#include "minwinbase.h"
#include "stdarg.h"
#include "time.h"

#define DEFAULT_BUFFER_LEN 512
#define DEFAULT_MAX_STR_LEN 128
#define DEFAULT_SERVER_IP "127.0.0.1"
#define DEFAULT_PORT "32768"


SOCKET ClientSocketInit_UDP(char* serverIP, char* serverPort);
char *dataInput(int lim, int flag);
char *packageEncoder(char *username, char *passwd, const char* cmd);
char *getPackageData(char *flag);
int sendMessage_UDP(SOCKET s, const char *buff, int len, int flags,
                    struct sockaddr *from, const int *fromLen);

void logPrintf(int flag, const char *format, ...);


#endif //TYPING_BATTLE_C_NET_H
