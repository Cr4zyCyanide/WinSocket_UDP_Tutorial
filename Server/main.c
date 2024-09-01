//C Standard: C11
//Windows Socket Version: 2.2
//IDE: Clion 2023.2
//C/S: Server
//Project: Typing Battle Server

//接收客户端用户注册和登陆
//接收用户的打字速度并排名,排名应该记录到文件中
//对挑战结果进行实时判定,如果挑战失败,将结果发往客户端,通知挑战终止
//发送打字内容到客户端

#include "files.h"
#include "net.h"
#include "threads.h"
#include "game.h"

#pragma comment (lib,"ws2_32.lib")
#pragma comment (lib,"Mswsock.lib")
#pragma comment (lib,"AdvApi32.lib")


int __cdecl main(int argc,char** argv) {
    //process the arguments
    if(argc != 2)
        printf("\nusage: ./typing_battle_S.exe [listen_port]\n");

    //get parameters the authing thread needs
    THREAD_PARAS t_authing_Paras;

    //initialization the ranking list
    ptr_rksHead head =(ptr_rksHead)malloc(sizeof (playerData));
    head->next = NULL;

    //head = rksList_Refresh(head);
    t_authing_Paras.head = head;

    strcpy(t_authing_Paras.serverIP, "127.0.0.1");
    logPrintf(1, "server started successfully at default address: %s.\n",
              t_authing_Paras.serverIP);

    //get port from argument
    if(argc == 1){
        logPrintf(1, "No port specified, using default port: 32768.\n");
        strcpy(t_authing_Paras.listeningPort, DEFAULT_PORT);
    }
    else
        strcpy(t_authing_Paras.listeningPort, argv[1]);

    //reserve
    HANDLE event_authingThreadEnds =
            CreateEvent(NULL, FALSE, FALSE,
                        "event_authingThreadEnds");
    //create authing server thread
    PTR_THREAD_PARAS ptr_t_authing_Paras = &t_authing_Paras;
    HANDLE t_handle_PlayerAuthing =
            CreateThread(NULL, 0, t_playerAuthing,
                         (void *) ptr_t_authing_Paras, 0, NULL);
    logPrintf(1, "[thread]authing thread start successfully.\n");

    WaitForSingleObject(event_authingThreadEnds, INFINITE);

    WaitForSingleObject(t_handle_PlayerAuthing, INFINITE);

    return 0;
}