//
// Created on 2023/11/21.
//
#include "stdio.h"
#include "stdarg.h"
#include "time.h"
#include "string.h"

#include "files.h"
#include "net.h"
#include "game.h"

void logPrintf(int flag, const char *format, ...) {
    //pointer to logfile
    FILE *pLogFile = fopen("log.txt", "a+");
    if (pLogFile == NULL) {
        printf("[error][files]can not create a log file(log.txt).\n");
        return;
    }
    //get current time str
    time_t currentTime;
    struct tm *timeInfo;
    char timeStr[20];
    time(&currentTime);
    timeInfo = localtime(&currentTime);
    strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M:%S", timeInfo);

    //variable arguments list
    va_list args;
    va_start(args, format);

    //1:information, 2:error
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

int readUserInfo_FromFile(char *username, char *passwd, int flag){
    FILE *file = fopen("data.ini", "r+");

    char existingUsername[DEFAULT_BUFFER_LEN];
    char existingPassword[DEFAULT_BUFFER_LEN];

    while (fscanf(file, "%s %s", existingUsername, existingPassword) == 2) {
        //login
        if (flag == 1){
            if(strcmp(existingUsername, username) == 0 && strcmp(existingPassword, passwd) == 0){
                return 1;
            }
        }
        //registry
        else if (flag == 2){
            if (strcmp(existingUsername, username) == 0)
                return 1;
        }
    }
    return 0;
}

int writeUserInfo_IntoFile(char *username, char *passwd){
    FILE *file = fopen("data.ini", "a+");

    if (file == NULL) {
        file = fopen("data.ini", "a+");
        if (file == NULL) {
            printf("[error]can not create a new file.(data.ini)\n");
            return -1;
        }
    }
    if (readUserInfo_FromFile(username, passwd, 2)) {
        logPrintf(2, "username already exists.\n");
        fclose(file);
        return 1;
    }
    else {
        fprintf(file, "%s %s\n", username, passwd);
        logPrintf(1, "user data are written into file successfully.\n");
        fclose(file);
        return 0;
    }
}

ptr_rksHead readRksInfo_FromFile(ptr_rksHead head){
    FILE *file = fopen("rks.txt", "r+");

    if(file == NULL){
        logPrintf(2, "can not find the file:rks.txt.\n");
        FILE *c_file = fopen("rks.txt", "a+");
        if (c_file == NULL) {
            printf("[error]can not create a new file(rks.txt).\n");
            return 0;
        }
        system("pause");
    }

    playerData temp;
    int cnt = 0;

    while (fscanf(file, "%s %s %s %s %s",
                  temp.username, temp.timeUsed, temp.avgSpeed, temp.acc, temp.score) == 5) {
        // allocate memory space and insert it into the list
        ptr_node newNode = (ptr_node)malloc(sizeof(playerData));
        if (newNode == NULL) {
            logPrintf(2, "Memory allocation failed.\n");
            exit(EXIT_FAILURE);
        }

        strcpy(newNode->username, temp.username);
        strcpy(newNode->timeUsed, temp.timeUsed);
        strcpy(newNode->acc, temp.acc);
        strcpy(newNode->avgSpeed, temp.avgSpeed);
        strcpy(newNode->score, temp.score);
        newNode->next = NULL;

        // check if the username already exists in the list
        if (usernameExists(head, temp.username)) {
            // update the record with the highest score for the existing username
            //logPrintf(1, "[file]a username exists:%s.\n", temp.username);
            updateHighestScore(head, newNode);
            // free the allocated memory since the node is not needed
            free(newNode);
        } else {
            // insert the new node into the list
            insertByScore(head, newNode);
        }

        cnt++;
    }

    fclose(file);

    if (cnt == 0)
        logPrintf(1, "[game]Cannot find any records.\n");
    else
        logPrintf(1, "[game]Found %d record(s).\n", cnt);

    return head;
}

int writeNewRecord_IntoFile(ptr_node newNode){
    FILE *file = fopen("rks.txt", "a+");
    if (file == NULL) {
        file = fopen("rks.txt", "a+");
        if (file == NULL) {
            printf("[error]can not create a new file(rks.txt).\n");
            return 0;
        }
    }

    fprintf(file, "%s %s %s %s %s\n",
            newNode->username, newNode->timeUsed, newNode->avgSpeed, newNode->acc, newNode->score);
    logPrintf(1, "[files]new records written in rks.txt:%s,%s,%s,%s,%s.\n",
              newNode->username, newNode->timeUsed, newNode->avgSpeed, newNode->acc, newNode->score);

    fclose(file);

    return 1;
}