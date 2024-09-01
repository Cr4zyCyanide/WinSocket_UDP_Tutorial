//
// Created on 2023/11/21.
//
#include "net.h"
#include "files.h"

//pkg structure:              pos
//1Byte PackageSize           0
//1Byte Command               1
//1Byte pointer to ZeroByte   2
//indeterminate size username 3~
//1Byte 0                     <-ptr
//indeterminate size passwd   <-ptr+1~
//1Byte 0
REQUEST *packageDecoder(const char *pkg, REQUEST *data){
    int ptr_ZeroByte = (int)pkg[2];
    char *username = (char*) malloc(strlen(&pkg[3])),
            *passwd = (char*) malloc(strlen(&pkg[ptr_ZeroByte + 1]));

    strcpy(username, (char*) &pkg[3]);
    strcpy(passwd, (char*) &pkg[1 + ptr_ZeroByte]);

    if(pkg[1] == '1')
        data->cmd = 1;
    else if(pkg[1] == '2')
        data->cmd = 2;
    data->ptr_username = username;
    data->ptr_passwd = passwd;

    return data;
}

SOCKET SocketInit_UDP(char* ip, char* port){
    SOCKET serverSocket = INVALID_SOCKET;
    int iResult;
    WSADATA wsaData;
    do{
        iResult = WSAStartup(MAKEWORD(2,2),&wsaData);
        if(iResult != 0){
            logPrintf(2, "[socket]WSAStartup failed with error code:%d\n",iResult);
            continue;
        }

        struct addrinfo* result = NULL, hints;
        ZeroMemory(&hints, sizeof (hints));
        hints.ai_family = AF_INET;
        hints.ai_protocol = IPPROTO_UDP;
        hints.ai_socktype = SOCK_DGRAM;
        hints.ai_flags = AI_PASSIVE;

        iResult = getaddrinfo(ip, port, &hints, &result);

        if(iResult != 0){
            logPrintf(2, "[socket]get address info failed with error code:%d\n",iResult);
            WSACleanup();
            continue;
        }
        serverSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);

        if(serverSocket == INVALID_SOCKET){
            logPrintf(2,"[socket]socket creation failed with error code:%d\n",WSAGetLastError());
            closesocket(serverSocket);
            freeaddrinfo(result);
            WSACleanup();
            continue;
        }
        //bind
        iResult = bind(serverSocket, result->ai_addr, (int)result->ai_addrlen );
        if(iResult == SOCKET_ERROR){
            logPrintf(2, "[socket]bind failed with error code:%d\n",WSAGetLastError());
            freeaddrinfo(result);
            closesocket(serverSocket);
            WSACleanup();
            continue;
        }
        freeaddrinfo(result);

    } while (serverSocket == INVALID_SOCKET);
    //socket create successfully
    //get the ip addr and port the socket just created using
    struct sockaddr_storage addr;
    int addrLen = sizeof(addr);

    getsockname(serverSocket, (struct sockaddr*)&addr, &addrLen);
    struct sockaddr_in *s = (struct sockaddr_in *)&addr;
    logPrintf(1, "[socket]new udp socket %d created at: %s:%d.\n",
              serverSocket, inet_ntoa(s->sin_addr), ntohs(s->sin_port));

    return serverSocket;
}

int sendMessage_UDP(SOCKET s, const char *buff, int len, int flags, struct sockaddr *to, const int *ptr_toLen){
    int iSendResult;
    //printf("========trying to send========\n");
    iSendResult = sendto(s, buff, len, flags, to, *ptr_toLen);
    //printf("========tried to send========\n");
    if(iSendResult == SOCKET_ERROR){
        logPrintf(2,"[socket]send failed with error code:%d\n",WSAGetLastError());
    }
    struct sockaddr_in sa_in = *(struct sockaddr_in*)to;
    logPrintf(1, "[socket]%d byte(s) has sent to %s:%d by using socket %d.\n", iSendResult,
              inet_ntoa(sa_in.sin_addr), ntohs(sa_in.sin_port), s);
    logPrintf(1, "[socket]text:%s\n", buff);
    return iSendResult;
}

int recvMessage_UDP_lim(SOCKET s,
                        char* buff,
                        int len,
                        int flag,
                        struct sockaddr_in *from,
                        int* ptr_fromLen,
                        ptr_addrinfo_client lim){
    int iRecvResult;
    char clientIP_str[INET_ADDRSTRLEN];
    char clientPort_str[6];

    u_short clientPort_us;
    while(1){
        iRecvResult = recvfrom(s, buff, len, flag, (SOCKADDR*)from, ptr_fromLen);
        inet_ntop(AF_INET, &(from->sin_addr),clientIP_str, sizeof(clientIP_str));
        clientPort_us = ntohs(  from->sin_port);
        snprintf(clientPort_str, sizeof(clientPort_str), "%d", clientPort_us);

        //msg received
        if(iRecvResult > 0){
            if((strcmp(clientIP_str, lim->clientIP) == 0) && (strcmp(clientPort_str, lim->clientPort) == 0)){
                logPrintf(1, "[socket]%d byte(s) received from %s:%s, limited: %s:%s\n", iRecvResult,
                          clientIP_str, clientPort_str, lim->clientIP, lim->clientPort);
                return iRecvResult;
            }
            ZeroMemory(buff, DEFAULT_BUFFER_LEN);
        }
    }
}

