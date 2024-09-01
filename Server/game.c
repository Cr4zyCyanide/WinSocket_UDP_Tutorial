//
// Created on 2023/11/21.
//


#include "game.h"
#include "files.h"
#include "net.h"

ptr_rksHead rksList_Refresh(ptr_rksHead head){
    //freeList(head);
    //read rks info from file and sort it by the score
    head = readRksInfo_FromFile(head);
    //traverse the linked list
    ptr_node p = head->next;
    //update numbers
    int no = 1;
    while (p != NULL){
        char rks_str[4];
        snprintf(rks_str, sizeof (rks_str), "%d", no);
        strcpy(p->no, rks_str);
        no++;
        p = p->next;
    }
    logPrintf(1, "[game]list refreshed.\n");
    return head;
}


void insertByScore(ptr_rksHead head, ptr_node newNode){

    double newScore = strtod(newNode->score, NULL);
    ptr_node current = head;
    while (current->next != NULL &&
           strtod(current->next->score, NULL) > newScore) {
        current = current->next;
    }
    newNode->next = current->next;
    current->next = newNode;
}

void rksPrint(ptr_rksHead head){
    ptr_node p = head->next;
    logPrintf(1, "=========current rankings=========\n");
    logPrintf(1, "No  Username   TimeUsed  AvgSpeed  acc  score\n");
    while(1){
        logPrintf(1, "%s     %s      %s     %s     %s    %s\n",
                  p->no, p->username, p->timeUsed, p->acc, p->avgSpeed, p->score);
        p = p->next;
        if(p == NULL)
            break;
    }
    logPrintf(1, "=========ranking ends=========\n");
}

//if server got a new record
void updateRanking(ptr_rksHead head, ptr_playerData newNode){
    //1.writeNewRecord_IntoFile
    writeNewRecord_IntoFile(newNode);
    logPrintf(1, "[game]before:\n");
    rksPrint(head);
    ptr_node p;
    //2.regenerate the ranking list
    //freeList(head);
    readRksInfo_FromFile(head);
    logPrintf(1, "[game]after:\n");
    rksPrint(head);
    /*
    //2.1check if the username already exists in ranking list
    int flag = 0;
    p = head->next;
    while (p){
        if(strcmp(p->username, newNode->username) == 0){
            flag = 1;
        }
        p = p->next;
    }
    //not exists
    if(flag == 0)
        insertByScore(head, newNode);
    //already exists
    else{

    */
    //traverse the linked list
    p = head->next;
    //update numbers
    int no = 1;
    while (p){
        char *rks_str = (char*)malloc(4);
        snprintf(rks_str, sizeof (rks_str), "%d", no);
        strcpy(p->no, rks_str);
        no++;
        p = p->next;
    }
    logPrintf(1, "[game]new record updated:%s,%s,%s,%s,%s.\n",
              newNode->username, newNode->timeUsed,
              newNode->avgSpeed, newNode->acc,
              newNode->score);
    //rksPrint(head);
}

ptr_node getTargetInfo(ptr_rksHead head, char* no){
    ptr_node p = (ptr_node)malloc(sizeof(playerData ));
    p = head;
    while (p){
        if(strcmp(p->no, no) == 0){
            logPrintf(1, "got challenge info: player:%s, time used:%s, score:%s.\n",
                      p->username, p->timeUsed, p->score);
            return p;
        }
        p = p->next;
    }
    logPrintf(2, "can not get userinfo in the ranking list:No.%s.\n", no);
    return NULL;
}

void printAndNumberList(ptr_node head) {
    int no = 1;
    ptr_node current = head->next;
    while (current != NULL) {
        snprintf(current->no, sizeof(current->no), "%d", no++);
        printf("No: %s, Username: %s, Score: %s\n", current->no, current->username, current->score);
        current = current->next;
    }
}

//check if a username already exists in the list
bool usernameExists(ptr_rksHead head, const char *username) {
    ptr_node current = head->next;
    while (current != NULL) {
        // Username already exists in the list
        if (strcmp(current->username, username) == 0) {
            return true;
        }
        current = current->next;
    }
    // Username not found in the list
    return false;
}

//update the record with the highest score for a given username
void updateHighestScore(ptr_rksHead head, ptr_node newNode) {
    ptr_node current = head->next;
    while (current != NULL) {
        //logPrintf(1, "current:%s, newNode:%s.\n", current->username, newNode->username);
        if (strcmp(current->username, newNode->username) == 0) {
            // Update the score if the new score is higher

            double currentScore = strtod(current->score, NULL);
            if (strtod(newNode->score, NULL) > currentScore) {
                logPrintf(1, "[game]tyring to update for %s.\n", newNode->username);
                //ZeroMemory(current->timeUsed, 16);
                strcpy(current->timeUsed, newNode->timeUsed);
                //ZeroMemory(current->avgSpeed, 16);
                strcpy(current->avgSpeed, newNode->avgSpeed);
                //ZeroMemory(current->acc, 16);
                strcpy(current->acc, newNode->acc);
                //ZeroMemory(current->score, 16);
                strcpy(current->score, newNode->score);
                logPrintf(1, "[game]updated score for %s, new score:%s.\n",
                          newNode->username, newNode->score);
            }
            return;
        }
        current = current->next;
    }
}

char* getRksAsString(ptr_rksHead head){
    //rksPrint(head);
    ptr_node p = head->next;
    char *rksString;
    /*
    //get strSize of memory the str needs
    size_t strSize;

    while (p){
        strSize = strSize +
                  strlen(p->no) +
                  strlen(p->username) +
                  strlen(p->timeUsed) +
                  strlen(p->acc) +
                  strlen(p->avgSpeed) +
                  strlen(p->score);
        p = p->next;
    }

    logPrintf(1, "[game]generate rks string needs %d bytes memory.\n", strSize);
    rksString = (char*)malloc(strSize + 64);
    */
    rksString = (char*)malloc(2048);
    ZeroMemory(rksString, 2048);
    if(rksString == NULL){
        logPrintf(2, "Memory allocation failed.\n");
        exit(EXIT_FAILURE);
    }



    p = head->next;
    while(p){
        strcat(rksString, "No.");
        strcat(rksString, p->no);
        strcat(rksString, " ");
        strcat(rksString, p->username);
        strcat(rksString, " ");
        strcat(rksString, p->timeUsed);
        strcat(rksString, "s");
        strcat(rksString, " ");
        strcat(rksString, p->acc);
        strcat(rksString, "%");
        strcat(rksString, " ");
        strcat(rksString, p->avgSpeed);
        strcat(rksString, "tabs/s");
        strcat(rksString, " ");
        strcat(rksString, p->score);
        strcat(rksString, "\n");
        //every record is split by '\n'
        p = p->next;
    }
    logPrintf(1, "[game]get ranking string successfully.\n");
    return rksString;
}

void freeList(ptr_node head) {
    ptr_node current = head;
    while (current != NULL) {
        ptr_node temp = current;
        current = current->next;
        free(temp);
    }
}

char *randString(int length){
    char *str = (char*)malloc(sizeof(char) * (length + 1));
    srand((unsigned int)time(NULL) * 12);
    int i;
    for(i = 0; i < length; i++){
        if(rand() % 4)
            str[i] = 'a' + rand() % 26;
        else
            str[i] = 'A' + rand() % 26;
    }
    str[length] = '\0';
    return str;
}

double scoreCalculator(double acc, double timeUsed){
    if (250 > timeUsed && timeUsed > 100)
        return (2500 - (double ) timeUsed * 5) * acc * acc;
    else if (timeUsed <= 100 &&timeUsed > 0 )
        return (10000 - (double ) timeUsed * 75) * acc;
    else
        return -1;
}

