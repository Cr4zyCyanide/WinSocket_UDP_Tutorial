//
// Created on 2023/11/21.
//

#ifndef TYPING_BATTLE_S_NET_H
#define TYPING_BATTLE_S_NET_H

#include "winsock2.h"
#include "ws2tcpip.h"
#include "minwinbase.h"

#include "game.h"
#include "threads.h"

#define DEFAULT_BUFFER_LEN 512
#define DEFAULT_PORT "32768"


typedef ADDRINFO_SERVER addrinfo_server;
typedef PTR_ADDRINFO_SERVER ptr_addrinfo_server;
typedef ADDRINFO_CLIENT addrinfo_client;
typedef PTR_ADDRINFO_CLIENT ptr_addrinfo_client;


typedef struct CLIENT_REQUEST{
    int cmd;
    char *ptr_username;
    char *ptr_passwd;
}REQUEST;

SOCKET SocketInit_UDP(char* ip, char* port);
REQUEST *packageDecoder(const char *pkg, REQUEST *data);

int sendMessage_UDP(SOCKET s, const char *buff, int len, int flags,
                    struct sockaddr *to, const int *ptr_toLen);
int recvMessage_UDP_lim(SOCKET s,char* buff,int len,int flag,
                        struct sockaddr_in *from,int* ptr_fromLen,
                        ptr_addrinfo_client lim);

#endif //TYPING_BATTLE_S_NET_H
