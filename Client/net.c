//
// Created on 2023/11/22.
//


#include "net.h"

//pkg structure:              pos
//1Byte PackageSize           0
//1Byte Command               1
//1Byte pointer to ZeroByte   2
//indeterminate size username 3~
//1Byte 0                     <-ptr
//indeterminate size passwd   <-ptr+1~
//1Byte 0
char *packageEncoder(char *username, char *passwd, const char* cmd){
    unsigned char size_u = (int)strlen(username), size_p = (int)strlen(passwd);
    unsigned char ptr_ZeroByte = size_u + 3;
    unsigned char size = size_u + size_p + 5;
    unsigned char packageSize = (unsigned char)size, zeroByte = 0;
    unsigned char *data = (unsigned char*)malloc(size);
    ZeroMemory(data, size);

    //gather the strings up
    data[0] = packageSize;
    data[1] = *cmd;
    data[2] = ptr_ZeroByte;

    strcat((char*)&data[3], username);
    strcat((char*)&data[ptr_ZeroByte],(const char*)&zeroByte);
    strcat((char*)&data[ptr_ZeroByte + 1],passwd);
//    printf("[debug]size:%d, size of username:%d, size of password:%d\n", size, size_u, size_p);

    return (char*)data;
}

SOCKET ClientSocketInit_UDP(char* serverIP, char* serverPort){
    SOCKET newSocket = INVALID_SOCKET;
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

        iResult = getaddrinfo(serverIP, NULL, &hints, &result);

        if(iResult != 0){
            logPrintf(2, "[socket]get address info failed with error code:%d\n",iResult);
            WSACleanup();
            continue;
        }
        newSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);

        if(newSocket == INVALID_SOCKET){
            logPrintf(2,"[socket]socket creation failed with error code:%d\n",WSAGetLastError());
            closesocket(newSocket);
            freeaddrinfo(result);
            WSACleanup();
            continue;
        }

        //bind
        iResult = bind(newSocket, result->ai_addr, (int)result->ai_addrlen);
        if(iResult == SOCKET_ERROR){
            logPrintf(2, "[socket]bind failed with error code:%d\n",WSAGetLastError());
            freeaddrinfo(result);
            closesocket(newSocket);
            WSACleanup();
            continue;
        }
        freeaddrinfo(result);

    } while (newSocket == INVALID_SOCKET);
    //socket create successfully
    //get the serverIP addr and serverPort the socket just created using
    struct sockaddr_storage addr;
    int addrLen = sizeof(addr);

    getsockname(newSocket, (struct sockaddr*)&addr, &addrLen);
    struct sockaddr_in *s = (struct sockaddr_in *)&addr;
    logPrintf(1, "[socket]new udp socket created at: %s:%d.\n",
              inet_ntoa(s->sin_addr), ntohs(s->sin_port));

    return newSocket;
}

char *dataInput(int lim, int flag){
    char *temp_str = (char*)malloc(1024);
    while(1){
        ZeroMemory(temp_str, DEFAULT_MAX_STR_LEN);
        if(flag == 1)
            printf("Username:");
        else if(flag == 2)
            printf("Password:");

        scanf("%s",temp_str);

        if(strlen(temp_str) >= lim){
            if(flag == 1)
                printf("Username");
            else if(flag == 2)
                printf("Password");
            printf(" can not longer than 64 characters.Please try again.\n");
            continue;
        }
        else{
            char *str = (char*)malloc(strlen(temp_str));
//            printf("%lld", strlen(temp_str));
            strcpy(str, temp_str);
            free(temp_str);
            return str;
        }
    }
}


char *getPackageData(char *flag){
    char *username, *passwd, *datagram;
    username = dataInput(DEFAULT_MAX_STR_LEN, 1);
    passwd = dataInput(DEFAULT_MAX_STR_LEN, 2);
    datagram = packageEncoder(username, passwd, flag);
    return datagram;
}

int sendMessage_UDP(SOCKET s, const char *buff, int len, int flags, struct sockaddr *from, const int *fromLen){
    int iSendResult;
    iSendResult = sendto(s, buff, len, flags, from, *fromLen);
    if(iSendResult == SOCKET_ERROR){
        printf("[socket]send failed with error code:%d\n",WSAGetLastError());
        closesocket(s);
        WSACleanup();
    }
    printf("[socket]bytes sent:%d,text:%s\n", iSendResult, buff);
    return iSendResult;
}

void logPrintf(int flag, const char *format, ...) {
    FILE *pLogFile = fopen("log.txt", "a+");
    if (pLogFile == NULL) {
        printf("[error][files]can not create a log file(log.txt).\n");
        return;
    }

    time_t currentTime;
    struct tm *timeInfo;
    char timeStr[20];
    time(&currentTime);
    timeInfo = localtime(&currentTime);
    strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M:%S", timeInfo);

    va_list args;
    va_start(args, format);

    if (flag == 1) {
        printf("[%s][info]", timeStr);
        fprintf(pLogFile, "[%s][info]", timeStr);
    } else {
        printf("[%s][error]", timeStr);
        fprintf(pLogFile, "[%s][error]", timeStr);
    }

    vfprintf(stdout, format, args);
    vfprintf(pLogFile, format, args);

    va_end(args);
    fclose(pLogFile);
}