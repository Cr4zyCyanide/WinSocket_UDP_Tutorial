//
// Created on 2023/11/22.
//
#include "time.h"
#include "winsock2.h"
#include "ws2tcpip.h"
#include "minwinbase.h"


#include "net.h"
#include "files.h"
#include "game.h"
#include "threads.h"

DWORD WINAPI t_playerAuthing(LPVOID lp_paras){
    Sleep(200);

    //Server UDP Socket creation
    PTR_THREAD_PARAS ptr_lp_paras = (PTR_THREAD_PARAS)lp_paras;
    THREAD_PARAS paras_thisThread = *(PTR_ADDRINFO_SERVER)lp_paras;

    SOCKET authingSocket = SocketInit_UDP(paras_thisThread.serverIP, paras_thisThread.listeningPort);

    HANDLE thisThreadEnds = OpenEvent(EVENT_MODIFY_STATE, FALSE,
                                               "event_authingThreadEnds");
    char requestBuffer[DEFAULT_BUFFER_LEN];
    char sendBuffer[DEFAULT_BUFFER_LEN];

    int iResult;
    int isStartThread;
    THREAD_PARAS paras_gameThread;
    paras_gameThread.head = ptr_lp_paras->head;


    char optVal[DEFAULT_BUFFER_LEN];
    int NONE_SocketError = 1;

    //new sockaddr initialization
    struct sockaddr_in clientAddr;
    int clientLen = sizeof(clientAddr);
    char clientIP_str[INET_ADDRSTRLEN];

    setsockopt(authingSocket, SOL_SOCKET, SO_REUSEADDR, optVal, DEFAULT_BUFFER_LEN);
    while(NONE_SocketError){

//        //initialization the ranking list
//        ptr_rksHead head =(ptr_rksHead)malloc(sizeof (playerData));
//        head->next = NULL;

        isStartThread = 0;

        //request processor
        REQUEST *pRequest = (struct CLIENT_REQUEST*)malloc(sizeof (struct CLIENT_REQUEST));
        //listening and processing loop
        while(1){
            ZeroMemory(requestBuffer, sizeof (requestBuffer));
            ZeroMemory(&clientAddr, clientLen);
            ZeroMemory(clientIP_str, INET_ADDRSTRLEN);

            //receive datagram
            logPrintf(1, "[socket]now authing socket listening...\n");
            iResult = recvfrom(authingSocket, requestBuffer, DEFAULT_BUFFER_LEN,
                               0, (SOCKADDR*)&clientAddr, &clientLen);

            //get src ip address
            inet_ntop(AF_INET, &(clientAddr.sin_addr), clientIP_str, sizeof(clientIP_str));
            u_short clientPort_us = ntohs(clientAddr.sin_port);
            char clientPort_str[6];
            snprintf(clientPort_str, sizeof(clientPort_str), "%d", clientPort_us);
            logPrintf(1, "[socket]new request from %s:%s.\n", clientIP_str, clientPort_str);
            //if data received successfully
            if(iResult > 0) {
                //store the ip and port into the structure



                strcpy(paras_gameThread.clientIP, clientIP_str);
                strcpy(paras_gameThread.clientPort, clientPort_str);

                packageDecoder(requestBuffer,pRequest);
                logPrintf(1, "[socket]bytes received:%d\n", iResult);
                //print the original data
                char originData[(int)requestBuffer[0]];
                for(int i = 0;i < (int)requestBuffer[0]; i++){
                    if((int)requestBuffer[i] != 0)
                        originData[i] = requestBuffer[i];
                    else{
                        originData[i] = '0';
                    }
                }
                logPrintf(1, "Package original text:%s\n", originData);

                if(pRequest->cmd == 1)
                    logPrintf(1, "command:1(login)\n");
                else if (pRequest->cmd == 2)
                    logPrintf(1, "command:2(Register)\n");
                logPrintf(1, "username:%s, password:%s\n"
                        ,pRequest->ptr_username,pRequest->ptr_passwd);
                //if datagram is valid, return a value to client
                ZeroMemory(sendBuffer,sizeof(sendBuffer));
                if(pRequest->cmd == 1 || pRequest->cmd == 2){
                    if(pRequest->cmd == 1){
                        if(readUserInfo_FromFile(pRequest->ptr_username, pRequest->ptr_passwd, 1)){
                            //login succeed
                            strcpy(sendBuffer,"1");
                            sendMessage_UDP(authingSocket, sendBuffer, (int) strlen(sendBuffer),
                                            0, (SOCKADDR *) &clientAddr, &clientLen);
                            logPrintf(1, "client login succeed from %s.\n", clientIP_str);
                            isStartThread = 1;
                        }
                        else{
                            //login failed
                            strcpy(sendBuffer,"11");
                            sendMessage_UDP(authingSocket, sendBuffer, (int) strlen(sendBuffer),
                                            0, (SOCKADDR *) &clientAddr, &clientLen);
                            logPrintf(1, "login failed from %s.\n", clientIP_str);
                        }
                    }
                    //registry
                    else if (pRequest->cmd == 2){
                        if(writeUserInfo_IntoFile(pRequest->ptr_username, pRequest->ptr_passwd)){
                            //registry failed
                            strcpy(sendBuffer,"12");
                            logPrintf(1, "registry failed from %s.\n", clientIP_str);
                            sendMessage_UDP(authingSocket, sendBuffer, (int) strlen(sendBuffer),
                                            0, (SOCKADDR *) &clientAddr, &clientLen);
                        }
                        else {
                            //registry succeed
                            strcpy(sendBuffer,"2");
                            sendMessage_UDP(authingSocket, sendBuffer, (int) strlen(sendBuffer),
                                            0, (SOCKADDR *) &clientAddr, &clientLen);
                            logPrintf(1, "registry succeed from %s.\n", clientIP_str);
                            isStartThread = 1;
                        }
                    }
                }
                else{
                    strcpy(sendBuffer,"10");
                    logPrintf(2, "unknown error.response:%d.\n", pRequest->cmd);
                    sendMessage_UDP(authingSocket, sendBuffer, (int) strlen(sendBuffer),
                                    0, (SOCKADDR *) &clientAddr, &clientLen);
                }
                //if client login or registry succeed, start the game thread

                if(isStartThread == 1){

                    strcpy(paras_gameThread.serverIP, paras_thisThread.serverIP);
                    strcpy(paras_gameThread.clientIP, clientIP_str);
                    strcpy(paras_gameThread.clientPort, clientPort_str);
                    strcpy(paras_gameThread.username, pRequest->ptr_username);
                    paras_gameThread.head = ptr_lp_paras->head;

                    PTR_THREAD_PARAS ptr_paras_t_game = &paras_gameThread;

                    logPrintf(1, "[thread]trying to create the game thread.\n");
                    HANDLE handle_gameThread = CreateThread(NULL, 0, t_ServerInGame,
                                                            (void *) ptr_paras_t_game, 0, NULL);
                    logPrintf(1, "new game thread started:%d.\n", handle_gameThread);
                }
                logPrintf(1, "[socket]authing request processing has done.\n");
                free(pRequest);
                //system("pause");
                break;
            }
            else if(iResult < 0){
                logPrintf(2, "iResult = %d, error code: %d.\n", iResult, WSAGetLastError());
                SetEvent(thisThreadEnds);
                NONE_SocketError = 0;

            }
        }
    }
}

DWORD WINAPI t_ServerInGame(LPVOID lp_paras){
    Sleep(200);

    DWORD normalExitCode = 1;
    PTR_THREAD_PARAS ptr_lp_paras = (PTR_THREAD_PARAS)lp_paras;
    THREAD_PARAS paras_thisThread = *(PTR_THREAD_PARAS)lp_paras;
    SOCKET gameSocket = SocketInit_UDP(paras_thisThread.serverIP, NULL);

    int getResult;
    struct addrinfo* getAddrResult_S = NULL, hints;
    getResult = getaddrinfo(paras_thisThread.clientIP, paras_thisThread.clientPort,
                            &hints, &getAddrResult_S);

    if(getResult != 0){
        logPrintf(2, "[socket]get address info failed with error code:%d\n",getResult);
        system("pause");
    }
    else{
        logPrintf(1, "get client ip and port for game socket:%s:%s.\n",
                  paras_thisThread.clientIP, paras_thisThread.clientPort);
    }

    /*
    //get opening port
    struct sockaddr_storage addr;
    int addrLen = sizeof(addr);

    getsockname(gameSocket, (struct sockaddr*)&addr, &addrLen);
    struct sockaddr_in *s = (struct sockaddr_in *)&addr;
    u_short gameSockPort_us = ntohs(s->sin_port);
    char gameSockPort_str[6];
    snprintf(gameSockPort_str, 6, "%d", gameSockPort_us);
    logPrintf(1, "[socket]game socket opened on %s:%d.\n",
              paras_thisThread->serverIP, gameSockPort_us);
    */

    char buffer[DEFAULT_BUFFER_LEN];
    ZeroMemory(buffer, sizeof (buffer));

    struct sockaddr_in clientAddr;
    int clientLen = sizeof(clientAddr);
    char clientIP[INET_ADDRSTRLEN];

    /*
    //0.send the listening port that game socket using to client
    strcpy(buffer, gameSockPort_str);
    sendMessage_UDP(gameSocket, buffer, DEFAULT_BUFFER_LEN, 0,
                    (SOCKADDR*)&clientAddr, &clientLen);
    */

    //1.send the ranking info to client
    //get rks string
    rksList_Refresh(ptr_lp_paras->head);
    char* rksString = getRksAsString(ptr_lp_paras->head);
    //char rksString[1024];
    //strcpy(rksString, getRksAsString(paras_thisThread.head));
    //printf("**********step 1************\n");
    sendMessage_UDP(gameSocket, rksString, (int)strlen(rksString), 0,
                    getAddrResult_S->ai_addr, (int*)&getAddrResult_S->ai_addrlen);
    //2.get client username and the player that the client want to challenge
    ZeroMemory(buffer, sizeof (buffer));
    //printf("**********step 2************\n");
    recvMessage_UDP_lim(gameSocket, buffer, DEFAULT_BUFFER_LEN, 0,
                        &clientAddr, &clientLen, &paras_thisThread);
    logPrintf(1, "[game]player %s from %s:%s want to challenge the No.%s player.\n",
              paras_thisThread.username, paras_thisThread.clientIP, paras_thisThread.clientPort, buffer);

    //3.get info of the challenge:time limitation and score
    //printf("**********step 3************\n");
    ptr_node target = getTargetInfo(paras_thisThread.head, buffer);
    if(target == NULL){
        logPrintf(2, "can not get challenge info from ranking list.\n");
        system("pause");
    }

    double targetScore_d = strtod(target->score, NULL);
    double timeLim_s = strtod(target->timeUsed, NULL);

    logPrintf(1, "[game]get the target %s for %s:%s.\n",
              target->username, paras_thisThread.clientIP, paras_thisThread.clientPort);

    //4.send a random string to the client
    strcpy(buffer, randString(MAX_STRING_LENGTH));
    sendMessage_UDP(gameSocket, buffer, MAX_STRING_LENGTH, 0,
                    (SOCKADDR *) &clientAddr, (const int *) &clientLen);
    logPrintf(1, "[game]generated a random str %s for client[%s:%s].\n",
              buffer, paras_thisThread.clientIP, paras_thisThread.clientPort);
    logPrintf(1, "[game]now wait the client send the ack msg.\n");

    //create the thread and event to control client gaming
    HANDLE event_accomplished = CreateEvent(NULL, 1, 0,
                                            "event_accomplished");
    THREAD_PARAS paras_t_listening;
    //needs to fix it....
    //do not reuse the socket
    //paras_t_listening.s = gameSocket;
    strcpy(paras_t_listening.listeningPort, paras_thisThread.clientPort);
    strcpy(paras_t_listening.serverIP, paras_thisThread.serverIP);
    strcpy(paras_t_listening.clientIP, paras_thisThread.clientIP);
    strcpy(paras_t_listening.clientPort, paras_thisThread.clientPort);

    HANDLE handle_listening =
            CreateThread(NULL, 0,
                         t_getClientGamingInfo, (void*)&paras_t_listening, 0, NULL);
    logPrintf(1, "[thread]in game listening thread has created: %d.\n",
              handle_listening);
    logPrintf(1, "[game]target of player from %s:%s : time:%.2f, score:%.2f.\n",
              paras_thisThread.clientIP, paras_thisThread.clientPort,
              timeLim_s, targetScore_d);


    //5.wait the client send the ack msg
    while(1){
        int ackResult;
        ZeroMemory(buffer, sizeof (buffer));
        ackResult = recvMessage_UDP_lim(gameSocket, buffer, DEFAULT_BUFFER_LEN, 0,
                                        (struct sockaddr_in *) &clientAddr, &clientLen, &paras_thisThread);
//        ackResult = recvMessage_UDP_lim(gameSocket, buffer, DEFAULT_BUFFER_LEN,0,
//                                        &clientAddr, &clientLen, &paras_thisThread);
        logPrintf(1, "[socket]wait for ack datagram.received: %s.\n", buffer);
        if(ackResult > 0){
            if(strcmp(buffer, "ready") == 0){
                logPrintf(1, "player from %s:%s is ready to challenge.\n",
                          paras_thisThread.clientIP, paras_thisThread.clientPort);
                break;
            }
        }
    }

    //6.start counting and listening
    clock_t startTime = clock();


    //this loop structure is used to checking the time player used
    int isAccomplished = 0;
    while(1){
        //Sleep(100);
        clock_t endTime = clock();
        double timeUsed_s = (double)(endTime - startTime) / CLOCKS_PER_SEC;
        if(timeUsed_s >= timeLim_s){
            //challenge timed out, send the datagram
            ZeroMemory(buffer, sizeof (buffer));
            strcpy(buffer, "Timed out.");
            logPrintf(1, "[game]client[%s:%s] challenge failed: timed out.\n", paras_thisThread.clientIP, paras_thisThread.clientPort);
            //terminate the listening thread
            sendMessage_UDP(gameSocket, buffer, sizeof(buffer), 0,
                            (SOCKADDR *) &clientAddr, (const int *) &clientLen);
            TerminateThread(handle_listening, normalExitCode);
            break;
        }

        if(WaitForSingleObject(event_accomplished, 5) == WAIT_OBJECT_0){
            //challenge ends
            isAccomplished = 1;
            logPrintf(1, "[game]client challenge accomplished.\n");
            TerminateThread(handle_listening, normalExitCode);
            break;
        }
    }

    if(isAccomplished){
        //get result from client
        ZeroMemory(buffer, sizeof (buffer));
        recvMessage_UDP_lim(gameSocket, buffer, DEFAULT_BUFFER_LEN,
                 0, (SOCKADDR_IN *) &clientAddr, &clientLen, &paras_thisThread);
        //process the result
        ptr_node ptr_newPlayerData = (ptr_node) malloc(sizeof (playerData ));

        //result package structure:
        //username + " " + timeUsed + " " + avgSpeed + " " + acc
        logPrintf(1, "[game]received result datagram original data:%s.\n", buffer);
        sscanf(buffer, "%s %s %s",
               ptr_newPlayerData->timeUsed, ptr_newPlayerData->avgSpeed, ptr_newPlayerData->acc);

        strcpy(ptr_newPlayerData->username, paras_thisThread.username);

        double acc = strtod(ptr_newPlayerData->acc, NULL);
        double tu = strtod(ptr_newPlayerData->timeUsed, NULL);
        double score_d = scoreCalculator(acc, tu);
        char score_str[16];

        snprintf(score_str, 16, "%.2f", score_d);
        strcpy(ptr_newPlayerData->score, score_str);

        logPrintf(1, "[game]client[%s:%s]: acc:%.2f, timeUsed:%.2f, score_str:%.2f.\n",
                  paras_thisThread.clientIP, paras_thisThread.clientPort, acc, tu, score_d);
        char resultStr[128];
        snprintf(resultStr, 128, "acc:%.2f, timeUsed:%.2f, score_str:%.2f.\n",acc, tu, score_d);

        if(score_d > targetScore_d){
            //succeed
            logPrintf(1, "[game]client[%s:%s] challenge succeed.\n",
                      paras_thisThread.clientIP, paras_thisThread.clientPort);
            //store the data as a node into the ranking list
            char scoreString[10];
            snprintf(scoreString, 10, "%.2f", score_d);
            strcpy(ptr_newPlayerData->score, scoreString);

            //send succeed datagram
            ZeroMemory(buffer, sizeof (buffer));
            strcpy(buffer, "You win!\n");
            strcat(buffer, resultStr);
            Sleep(2000);
            sendMessage_UDP(gameSocket, buffer, (int)strlen(buffer), 0,
                            (SOCKADDR *) &clientAddr, (const int *) &clientLen);

            updateRanking(ptr_lp_paras->head, ptr_newPlayerData);
            rksList_Refresh(ptr_lp_paras->head);
            //rksList_Refresh(paras_thisThread.head);
            logPrintf(1, "[game]Rks list refreshed.\n");
        }
        else{
            //failed
            logPrintf(1, "[game]player %s from client[%s:%s] challenge failed.\n",
                      paras_thisThread.username, paras_thisThread.clientIP, paras_thisThread.clientPort);

            //send failed datagram
            ZeroMemory(buffer, sizeof (buffer));
            strcpy(buffer, "Failed: Score is lower than the player you just challenged.\n");
            strcat(buffer, resultStr);
            Sleep(2000);
            sendMessage_UDP(gameSocket, buffer, (int)strlen(buffer), 0,
                            (SOCKADDR *) &clientAddr, (const int *) &clientLen);
        }
    }
    logPrintf(1, "[threads]listening thread for client[%s:%s] terminated with code:%ld.\n",
              paras_thisThread.clientIP, paras_thisThread.clientPort, normalExitCode);

    Sleep(5000);
    logPrintf(1, "[thread]ServerInGame thread exited. ExitCode:1.\n");
    closesocket(gameSocket);
    ExitThread(1);
}


DWORD WINAPI t_getClientGamingInfo(LPVOID lp_paras){
    Sleep(200);

    HANDLE event_accomplished = OpenEvent(EVENT_MODIFY_STATE, FALSE,
                                          "event_accomplished");

    THREAD_PARAS paras_thisThread = *(ptr_addrinfo_client)lp_paras;
    logPrintf(1, "[socket]trying to create inGame listen socket.\n");
    SOCKET gameSocket = SocketInit_UDP(paras_thisThread.serverIP, NULL);

    struct addrinfo* getAddrResult_S = NULL, hints;
    int iGetResult = getaddrinfo(paras_thisThread.clientIP, paras_thisThread.clientPort, &hints, &getAddrResult_S);

    if(iGetResult != 0){
        logPrintf(2, "[socket]get address info failed with error code:%d\n",iGetResult);
        system("pause");
    }

    char buffer[DEFAULT_BUFFER_LEN];

    ZeroMemory(buffer, sizeof (buffer));
    strcpy(buffer, "server is ready.");
    sendMessage_UDP(gameSocket, buffer, DEFAULT_BUFFER_LEN, 0,
                    getAddrResult_S->ai_addr, (const int *) &getAddrResult_S->ai_addrlen);

    struct sockaddr_in clientAddr;
    int clientLen = sizeof(clientAddr);

    while(1){
        ZeroMemory(buffer, sizeof (buffer));
        recvMessage_UDP_lim(gameSocket, buffer, DEFAULT_BUFFER_LEN, 0,
                            &clientAddr, &clientLen, &paras_thisThread);

        if(strcmp(buffer, "accomplished") == 0){
            logPrintf(1, "client[%s:%s] challenge accomplished.\n", paras_thisThread.clientIP, paras_thisThread.clientPort);
            closesocket(gameSocket);
            SetEvent(event_accomplished);
            break;
        }
        else{
            logPrintf(1, "client[%s:%s] current typing speed:%s.\n",
                      paras_thisThread.clientIP, paras_thisThread.clientPort, buffer);
        }
    }
    //logPrintf(1, "[thread]getClientGamingInfo thread exited. ExitCode:1.\n");
    //ExitThread(1);
}