//
// Created on 2023/11/22.
//

#ifndef TYPING_BATTLE_C_THREADS_H
#define TYPING_BATTLE_C_THREADS_H

#include "net.h"
#include "Windows.h"

#define FALSE 0
#define TURE 1

typedef struct THREAD_PARAMETERS{
    char serverIP[INET_ADDRSTRLEN];
    char serverPort[6];
    char clientIP[INET_ADDRSTRLEN];
    char clientPort[6];
    char* username;
    SOCKET s;
    //reserve
    HANDLE h_event;
}THREAD_PARAS ,ADDRINFO_SERVER, *PTR_ADDRINFO_SERVER, ADDRINFO_CLIENT, *PTR_ADDRINFO_CLIENT;

typedef ADDRINFO_SERVER addrinfo_server;
typedef PTR_ADDRINFO_SERVER ptr_addrinfo_server;
typedef ADDRINFO_CLIENT addrinfo_client;
typedef PTR_ADDRINFO_CLIENT ptr_addrinfo_client;
typedef THREAD_PARAS *PTR_THREAD_PARAS;

DWORD WINAPI t_playerLogin(LPVOID lp_paras);
DWORD WINAPI t_ClientInGame(LPVOID lp_paras);
DWORD WINAPI t_inGame(LPVOID lp_paras);

#endif //TYPING_BATTLE_C_THREADS_H
