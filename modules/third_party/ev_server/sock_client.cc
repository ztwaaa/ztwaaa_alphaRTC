// #include <windows.h>
// #include <winsock2.h>
// #include <iostream>

// #pragma comment(lib,"ws2_32.lib")
// SOCKET sockClient = socket(AF_INET, SOCK_STREAM, 0);
// const char * RequestBandwidthCommand = "RequestBandwidth";

// int main(char *argv)
// {
//     //unsigned long iMode=1;
//     //long iResult=0;
    
//     WORD wVersionRequested;//版本
// 	WSADATA wsaData;//库版本信息结构
// 	SOCKADDR_IN addrSrv;//服务器地址结构
//     int err;
    
	
//     int port = atoi(argv);
//     std::cout << "init" << std::endl;
// 	wVersionRequested = MAKEWORD(2, 2);//version:1.1
// 	err = WSAStartup(wVersionRequested, &wsaData);
// 	if (err != 0)
// 		exit(-1);
// 	if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2)
// 	{
// 		WSACleanup();//不符合则关闭套接字库
// 		exit(-2);
// 	}

// 	/*设置服务器地址*/
//     memset(&addrSrv, 0, sizeof(addrSrv));
    
// 	addrSrv.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
   
// 	addrSrv.sin_family = AF_INET;
// 	addrSrv.sin_port = htons(port);
// 	connect(sockClient, (struct sockaddr*)&addrSrv, sizeof(addrSrv));
	
//     std::cout << "init finished" << std::endl;
//     send(sockClient, RequestBandwidthCommand, strlen(RequestBandwidthCommand), 0);
//     char recvBuf[1024]="0";
//     recv(sockClient, recvBuf, sizeof(recvBuf), 0);
//     std::cout << "TEST:!!!!" << recvBuf <<std::endl;

//     return 0;
// }
#include <stdio.h>
#include <winsock2.h>

#pragma comment(lib,"ws2_32.lib")

static SOCKET socket_client;         //本地创建的客户端socket
static struct sockaddr_in server_in; //用于存储服务器的基本信息

const char * RequestBandwidthCommand = "RequestBandwidth";

int main(int argc, char* argv[])
{
    char   recData[255];          //这个地方一定要酌情设置大小，这决定了每次能获取多少数据
    int ret;                      //recv函数的返回值 有三种状态每种状态的含义在下方有解释

    WORD socket_version;
    WSADATA wsadata; 
    socket_version = MAKEWORD(2,2);
    if(WSAStartup(socket_version, &wsadata) != 0)
    {
        printf("WSAStartup error!");
        system("pause");
        return 0;
    }

    socket_client = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(socket_client == INVALID_SOCKET)
    {
        printf("invalid socket !");
        system("pause");
        return 0;
    }

    server_in.sin_family = AF_INET;    //IPV4协议族
    server_in.sin_port = htons(5000);  //服务器的端口号
    server_in.sin_addr.S_un.S_addr = inet_addr("127.0.0.1"); //服务IP
    if(connect(socket_client, (struct sockaddr *)&server_in, sizeof(server_in)) == SOCKET_ERROR)
    {
        printf("connect error\n");
        system("pause");
        return 0;
    }else{}

    printf("connect %s:%d\n", inet_ntoa(server_in.sin_addr), server_in.sin_port);
    send(socket_client, "hello i am client", strlen("hello i am client"), 0);
    while(1)
    {
        ret = recv(socket_client, recData, 255, 0);
        if(ret > 0)
        {
            printf("recData:%d",recData);
            send(socket_client, RequestBandwidthCommand, strlen(RequestBandwidthCommand), 0);

        }
        else if(ret == 0)
        {
            //当ret == 0 说明服务器掉线。
            printf("lost connection , Ip = %s\n", inet_ntoa(server_in.sin_addr));
            break;
        }
        else//ret < 0
        {
            //当ret < 0 说明出现了异常 例如阻塞状态解除，或者读取数据时出现指针错误等。
            //所以我们这里要主动断开和客户端的链接。
            printf("something wrong of %s\n", inet_ntoa(server_in.sin_addr));
            closesocket(socket_client);
            break;  
        }
    }

    closesocket(socket_client);
    WSACleanup();
    return 0;
}

