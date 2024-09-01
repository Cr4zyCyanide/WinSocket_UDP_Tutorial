//
// Created on 2023/11/22.
//

#ifndef TYPING_BATTLE_S_THREADS_H
#define TYPING_BATTLE_S_THREADS_H


#include "Windows.h"
#include "stdio.h"

#define FALSE 0
#define TURE 1

typedef struct THREAD_PARAMETERS{
    char serverIP[INET_ADDRSTRLEN];
    char listeningPort[6];
    char clientIP[INET_ADDRSTRLEN];
    char clientPort[6];
    char username[64];
    ptr_rksHead head;
    SOCKET s;
    //reserve
    HANDLE h_event;
}THREAD_PARAS ,ADDRINFO_SERVER, *PTR_ADDRINFO_SERVER, ADDRINFO_CLIENT, *PTR_ADDRINFO_CLIENT;

typedef THREAD_PARAS *PTR_THREAD_PARAS;

DWORD WINAPI t_playerAuthing(LPVOID lp_paras);
DWORD WINAPI t_ServerInGame(LPVOID lp_paras);
DWORD WINAPI t_getClientGamingInfo(LPVOID lp_paras);

#endif //TYPING_BATTLE_S_THREADS_H
