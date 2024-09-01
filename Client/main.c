//C Standard: C11
//Windows Socket Version: 2.2
//IDE: Clion 2023.2
//C/S: Client
//Project: Typing Battle Client

//用户注册和登陆
//显示当前所有用户排名
//选择要挑战的用户
//接收服务器打字内容
//打字开始,同时判断字符是否正确,并同时统计打字速度,发往服务器
//接收挑战结果

#include "threads.h"
#include "net.h"


#pragma comment (lib,"ws2_32.lib")
#pragma comment (lib,"Mswsock.lib")
#pragma comment (lib,"AdvApi32.lib")


int __cdecl main(int argc,char** argv) {
    if (argc != 2 && argc != 3)
        printf("\nusage: %s [server_address] [server_port]\n", argv[0]);

    THREAD_PARAS Paras_login;
    //get server port and ip address
    if (argc == 1) {
        strcpy(Paras_login.serverIP, DEFAULT_SERVER_IP);
        strcpy(Paras_login.serverPort, DEFAULT_PORT);
        logPrintf(1, "no server ip specified, using default server ip address: %s\n", DEFAULT_SERVER_IP);
        logPrintf(1, "no port specified, using default server port: %s\n", DEFAULT_PORT);
    } else if (argc == 2) {
        strcpy(Paras_login.serverIP, argv[1]);
        strcpy(Paras_login.serverPort, DEFAULT_PORT);
        logPrintf(1, "server ip address:%s\n[info]no port specified, using default server port:%s\n", argv[1],
                  DEFAULT_PORT);
    } else {
        strcpy(Paras_login.serverIP, argv[1]);
        strcpy(Paras_login.serverPort, argv[2]);
        logPrintf(1, "server ip address:%s, server port:%s\n", argv[1], argv[2]);
    }

    HANDLE t_handle_playerLogin =
            CreateThread(NULL, 0, t_playerLogin,
                                               (void *)&Paras_login, 0, NULL);

    WaitForSingleObject(t_handle_playerLogin, INFINITE);

    system("pause");
    return 0;

}