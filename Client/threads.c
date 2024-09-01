//
// Created on 2023/11/22.
//

#include "threads.h"
#include "net.h"
#include "game.h"

DWORD WINAPI t_playerLogin(LPVOID lp_paras){
    THREAD_PARAS paras_thisThread = *(PTR_ADDRINFO_SERVER)lp_paras;

    char receiveBuffer[DEFAULT_BUFFER_LEN];
    int iResult;

    SOCKET clientSocket = ClientSocketInit_UDP(paras_thisThread.serverIP, paras_thisThread.serverPort);

    struct sockaddr_in serverAddr;
    int clientLen = sizeof(serverAddr);

    int getResult;
    struct addrinfo* getAddrResult_S = NULL, hints;
    getResult = getaddrinfo(paras_thisThread.serverIP, paras_thisThread.serverPort, &hints, &getAddrResult_S);

    char serverIP_str[INET_ADDRSTRLEN];
    char serverPort_str[6];
    u_short serverPort_us;


    if(getResult != 0){
        logPrintf(2, "[socket]get address info failed with error code:%d\n",getResult);
        system("pause");
    }

    int flag;
    while(1){
        flag = 0;
        printf("================================\n"
               "Now you can log into the server.\n"
               "If don't have an account, you can register for one.\n");

        printf("1.Login.\n2.Register and login.\n");
        char *select_tmp = (char*) malloc(DEFAULT_MAX_STR_LEN), select;
        while(1){
            ZeroMemory(select_tmp, DEFAULT_MAX_STR_LEN);
            scanf("%s", select_tmp);
            if(strcmp(select_tmp, "1") == 0 || strcmp(select_tmp, "2") == 0 ){
                select = select_tmp[0];
                break;
            }
            else{
                printf("[error]invalid input:you can only input 1 or 2 but not others.\n");
                continue;
            }
        }
        free(select_tmp);
        //send request
        char *datagram = getPackageData(&select);


        iResult = sendto(clientSocket, datagram, (int)datagram[0], 0,
                         getAddrResult_S->ai_addr, (int)getAddrResult_S->ai_addrlen);

        if(iResult == SOCKET_ERROR){
            printf("[SOCKET_ERROR]send request failed with error code:%d\n",WSAGetLastError());
            system("pause");
        }

        printf("[info]%d Bytes were sent successfully.\n",iResult);
        //waiting for response of the server
        do{
            ZeroMemory(receiveBuffer,sizeof (receiveBuffer));
            iResult = recvfrom(clientSocket, receiveBuffer, DEFAULT_BUFFER_LEN, 0, NULL, NULL);
            if(iResult > 0){
                printf("received successfully.\nbytes received:%d,",iResult);
                printf("code:%s\n",receiveBuffer);

                inet_ntop(AF_INET, &(serverAddr.sin_addr), serverIP_str, sizeof(serverIP_str));
                serverPort_us = ntohs(serverAddr.sin_port);
                snprintf(serverPort_str, sizeof(serverPort_str), "%d", serverPort_us);

                //printLocalTime();
                if(strcmp(receiveBuffer,"1") == 0) {
                    printf("[info]Login successfully.\n");
                    flag = 1;
                    break;
                }
                else if(strcmp(receiveBuffer,"2") == 0){
                    printf("[info]Register and login successfully.\n");
                    flag = 1;
                    break;
                }
                else if(strcmp(receiveBuffer, "11") == 0){
                    printf("[error]username or password are incorrect or doesn't exist.\n"
                           "Please try again.\n");
                    break;
                }
                else if(strcmp(receiveBuffer, "12") == 0){
                    printf("[error]username already exists."
                           "please try again.\n");
                    break;
                }
                else
                    printf("[error]data received error.\n");
            }
        }while(1);

        //login succeed
//        ZeroMemory(receiveBuffer,sizeof (receiveBuffer));
//        iResult = recvfrom(clientSocket, receiveBuffer, DEFAULT_BUFFER_LEN, 0, NULL, NULL);
        if(flag){
            printf("[info]server received successfully.\n");
            //create client in game thread
            THREAD_PARAS Paras_inGame;
            strcpy(Paras_inGame.serverIP, serverIP_str);
            strcpy(Paras_inGame.serverPort, serverPort_str);
            Paras_inGame.s = clientSocket;
            HANDLE t_handle_InGame =
                    CreateThread(NULL, 0, t_ClientInGame,
                                 (void*)&Paras_inGame, 0, NULL);
            logPrintf(1, "[thread]in game thread created successfully:%d.\n", t_handle_InGame);
            WaitForSingleObject(t_handle_InGame, INFINITE);
            break;
        }
    }
}

DWORD WINAPI t_ClientInGame(LPVOID lp_paras){

    THREAD_PARAS paras_thisThread =*(PTR_THREAD_PARAS)lp_paras;
    char buffer[DEFAULT_BUFFER_LEN];
    char rksBuffer[1024];
    int recvResult;

    //1.reuse the login socket as inGame socket
    SOCKET inGameSocket = paras_thisThread.s;
    logPrintf(1, "[socket] tried to reuse the socket %d as inGameSocket %d.\n",
              paras_thisThread.s, inGameSocket);
    //2.1.receive the rks str to print and get server listen port for this game
    //create the struct to store the server port for this game
    struct sockaddr_in serverAddr;
    int serverLen = sizeof(serverAddr);

    //receive the rks str
    ZeroMemory(buffer, sizeof (buffer));
    recvfrom(inGameSocket, buffer, DEFAULT_BUFFER_LEN, 0,
             (SOCKADDR*)&serverAddr, &serverLen);
    if(recvResult < 0){
        logPrintf(2,"[socket]socket %d received failed with error code: %d.\n",
                  inGameSocket, WSAGetLastError());
        system("pause");
    }
    char serverIP_Str[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &(serverAddr.sin_addr), serverIP_Str, sizeof(serverIP_Str));
    u_short serverPort_us = ntohs(serverAddr.sin_port);

    logPrintf(1, "get server ip and port:%s:%d.\n", serverIP_Str, serverPort_us);
    //print it
    ZeroMemory(rksBuffer, 1024);
    strcpy(rksBuffer, buffer);
    logPrintf(1, "Current Ranking List:\nNo.|Username|Time Used|Accuracy|Typing Speed|Score\n%s", rksBuffer);

    //get number of players in the ranking list
    int cnt = 0;
    char* p = strtok(rksBuffer, "\n");
    while(p != NULL){
        cnt++;
        p = strtok(NULL, "\n");
    }

    //2.2choose a target that player want to challenge
    printf("Please input the number of the player on this list you wanna challenge.\n");
    char *select_tmp = (char*) malloc(DEFAULT_MAX_STR_LEN);
    int select;
    while(1){
        select = 0;
        ZeroMemory(select_tmp, DEFAULT_MAX_STR_LEN);
        scanf("%s", select_tmp);
        select = strtol(select_tmp, NULL, 10);
        if(select <= cnt && select >= 0){
            logPrintf(1, "[game]you have chosen no.%d as target.\n", select);
            break;
        }
        else{
            printf("[error]invalid input:you can only input 1 to %d but not others.\n", cnt);
            continue;
        }
    }
    free(select_tmp);

    //2.3.send the No of the player that the player want to challenge
    ZeroMemory(buffer, sizeof (buffer));
    char no_str[5];
    snprintf(no_str, 5, "%d", select);
    strcpy(buffer, no_str);
    sendMessage_UDP(inGameSocket, buffer, 5, 0,
                    (SOCKADDR*)&serverAddr, &serverLen);

    //3.receive a random string
    ZeroMemory(buffer, sizeof (buffer));
    recvfrom(inGameSocket, buffer, DEFAULT_BUFFER_LEN, 0,
             (SOCKADDR*)&serverAddr, &serverLen);
    if(recvResult < 0){
        logPrintf(2,"[socket]socket %d received failed with error code: %d.\n",
                  inGameSocket, WSAGetLastError());
        system("pause");
    }
    logPrintf(1, "[game]received random string.\n");
    char* randString = (char*)malloc(strlen(buffer) + 1);
    strcpy(randString, buffer);

    //4.create listening thread

    //4.1.get server in game listening port and check if server is ready

    struct sockaddr_in serverAddr_listen;
    int serverLen_listen = sizeof(serverAddr_listen);
    ZeroMemory(buffer, sizeof (buffer));


    logPrintf(1, "[game]now waiting for server gets ready.\n");

    recvResult = recvfrom(inGameSocket, buffer, DEFAULT_BUFFER_LEN, 0,
             (SOCKADDR*)&serverAddr_listen, &serverLen_listen);
    if(recvResult < 0){
        logPrintf(2,"[socket]socket %d received failed with error code: %d.\n",
                              inGameSocket, WSAGetLastError());
        system("pause");
    }
    logPrintf(1, "[socket]server ack datagram: %s.\n", buffer);

    //4.2.prepare all parameters TypingGame thread needs
    THREAD_PARAS paras_typingGame;
    paras_typingGame.s = inGameSocket;
    HANDLE t_handle_inGameListen =
            CreateThread(NULL, 0, t_inGame,
                         (void *) &paras_typingGame, 0, NULL);
    logPrintf(1, "[thread]inGameListen thread has created: %d\n", t_handle_inGameListen);

    //4.3.create an event to control the game
    HANDLE handle_eventTimedOut = CreateEvent(NULL, 1, 0,
                                              "event_timedOut");

    //4.4.send the ack datagram to start the challenge(ready)
    printf("Press ant key to start the game...\n");
    system("pause");

    ZeroMemory(buffer, sizeof (buffer));
    strcpy(buffer, "ready");
    logPrintf(1, "[game]send the ack datagram to server: %s.\n", buffer);
    sendMessage_UDP(inGameSocket, buffer, (int)strlen(buffer), 0,
                    (SOCKADDR*)&serverAddr, &serverLen);

    //5.game start
    int randStringLen = (int)strlen(randString);
    int isTimedOut = 1;

    system("cls");
    printf("Typing Game Start!\n");
    printf("target:\n%s\n", randString);
    printf("player input:\n");
    clock_t startTime = clock();
    clock_t currentTime;

    u_char userInput;
    int correctCount = 0;
    int totalCount = 0;
    u_char userInputString[50] = {0};

    double totalTime;
    double typingSpeed_ps;
    double typingSpeed_pm;
    double currentTimeUsed_s;
    double currentSpeed;
    double acc;

    do{

        if(WaitForSingleObject(handle_eventTimedOut, 1) == WAIT_OBJECT_0){
            //timed out
            isTimedOut = 1;
            TerminateProcess(t_handle_inGameListen, 1);
            logPrintf(1, "[thread]inGameListen thread terminated with code: 1.\n");
            break;
        }

        userInput = _getch();
        //printf("%d,%d\n",correctCount, totalCount);

        if (userInput == randString[correctCount]) {
            correctCount++;
            totalCount++;

            printf("%c", userInput);
            //refresh stdout buffer to print the char immediately
            fflush(stdout);
            userInputString[correctCount - 1] = userInput;
            //
            if (correctCount == randStringLen) {
                isTimedOut = 0;

                clock_t endTime = clock();
                totalTime = (double)(endTime - startTime) / CLOCKS_PER_SEC;
                typingSpeed_ps = (double)randStringLen / totalTime;
                typingSpeed_pm = typingSpeed_ps * 60;
                acc = ((double)correctCount / totalCount) * 100;

                //send accomplished datagram to server listening port
                ZeroMemory(buffer, DEFAULT_BUFFER_LEN);

                strcpy(buffer, "accomplished");
                sendMessage_UDP(inGameSocket, buffer, (int) strlen(buffer), 0,
                                (SOCKADDR*)&serverAddr_listen, &serverLen_listen);

                printf("\n***************\nTyping accomplished!\n");
                printf("Total time used: %.2lf seconds.\n", totalTime);
                printf("Typing Average Speed: %.2lf chars/min (%.2lf chars/sec).\n", typingSpeed_pm, typingSpeed_ps);
                printf("Incorrect inputs: %d.\n", totalCount - correctCount);
                printf("Accuracy: %.2f %%.\n", acc);
                break;
            }
        } else {
            system("cls");
            printf("Incorrect input!Please try again.\n%s\n", randString);
            printPlayerInput(correctCount, userInputString);
            totalCount ++;
        }

        /*
        //calculate current typing speed and send it to server
        currentTime = clock();
        currentTimeUsed_s = (double)(currentTime - startTime) / CLOCKS_PER_SEC;
        currentSpeed = correctCount / currentTimeUsed_s;
        snprintf(buffer, DEFAULT_BUFFER_LEN, "%.2f", currentSpeed);
        sendto(inGameSocket, buffer, DEFAULT_BUFFER_LEN, 0,
               (SOCKADDR*)&serverAddr, serverLen);
        */

    } while (1);

    //game over
    if(isTimedOut){
        logPrintf(1, "=======================\ngame over: timed out.\n");
    }
    else{
        logPrintf(1, "=======================\nchallenge accomplished, now send the data to server to check your score.\n");
        //6.if challenge not timed out, send the result datagram
        //result package structure:
        //username + " " + timeUsed + " " + avgSpeed + " " + acc
        Sleep(200);
        char resultString[256];
        snprintf(resultString, 256,"%.2f %.2f %.2f",
                 totalTime, typingSpeed_pm, acc);
        sendMessage_UDP(inGameSocket, resultString, (int)strlen(resultString), 0,
                        (SOCKADDR*)&serverAddr, &serverLen);
        logPrintf(1, "[game]your data has sent to server, original data:%s\n", resultString);

        //7.receive the result.
        ZeroMemory(buffer, DEFAULT_BUFFER_LEN);
        logPrintf(1, "[game]Now waiting for the response of the server.\n");
    }

    //8.get back to the last level or back to the login screen
    //developing...

    logPrintf(1, "[thread]ClientInGame thread exited. ExitCode:1.\n");
    ExitThread(1);
}

DWORD WINAPI t_inGame(LPVOID lp_paras){

    THREAD_PARAS paras_thisThread = *(PTR_THREAD_PARAS)lp_paras;
    SOCKET listenSocket = paras_thisThread.s;
    logPrintf(1, "[socket] tried to reuse the socket %d as listenSocket %d.\n",
              paras_thisThread.s, listenSocket);
    HANDLE handle_eventTimedOut = OpenEvent(EVENT_MODIFY_STATE, FALSE,
                                            "event_timedOut");
    char buffer[DEFAULT_BUFFER_LEN];

    struct sockaddr_in serverAddr;
    int serverLen = sizeof(serverAddr);
    int recvResult;
    logPrintf(1, "[socket]listening the timed out datagram...\n");

    while(1){
        recvResult = recvfrom(listenSocket, buffer, DEFAULT_BUFFER_LEN, 0,
                 (SOCKADDR*)&serverAddr, &serverLen);
        if(recvResult > 0){
            if(strcmp(buffer, "Timed out.") == 0){
                SetEvent(handle_eventTimedOut);
                logPrintf(1, "[game]Timed out.\n");
                break;
                //Sleep(INFINITE);
            } else{
                logPrintf(1, "[game]Server response:%s.\n", buffer);
                break;
            }
        } else{
            logPrintf(2, "[socket] received with error code:%d", WSAGetLastError());
        }
    }
    logPrintf(1, "[thread]InGame Listening thread exited. ExitCode:1.\n");
    ExitThread(1);
}

