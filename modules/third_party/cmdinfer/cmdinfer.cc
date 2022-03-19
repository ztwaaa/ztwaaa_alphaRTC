
   #include "cmdinfer.h"

#include "modules/third_party/statcollect/json.hpp"

#include <iostream>

/*以下为新增代码*/
#ifdef WIN32
#include<windows.h>
#include<winsock2.h>
#pragma comment(lib,"ws2_32.lib")
SOCKET sockClient = socket(AF_INET, SOCK_STREAM, 0);
#else
#include<netinet/in.h>
#include<unistd.h>
#include<sys/socket.h>
int sockClient = socket(AF_INET, SOCK_STREAM, 0);
#endif
const char * RequestBandwidthCommand = "RequestBandwidth";


void cmdinfer::ReportStates(
    std::uint64_t sendTimeMs,
    std::uint64_t receiveTimeMs,
    std::size_t payloadSize,
    std::uint8_t payloadType,
    std::uint16_t sequenceNumber,
    std::uint32_t ssrc,
    std::size_t paddingLength,
    std::size_t headerLength) {

    nlohmann::json j;
    j["send_time_ms"] = sendTimeMs;
    j["arrival_time_ms"] = receiveTimeMs;
    j["payload_type"] = payloadType;
    j["sequence_number"] = sequenceNumber;
    j["ssrc"] = ssrc;
    j["padding_length"] = paddingLength;
    j["header_length"] = headerLength;
    j["payload_size"] = payloadSize;

    std::string str = j.dump();
    const char* p =str.c_str();
    send(sockClient, p, strlen(p), 0);
    //std::cout << j.dump() << std::endl;
}

void cmdinfer::init_socket(char *argv)
{
    //unsigned long iMode=1;
    //long iResult=0;
    #ifdef WIN32
    WORD wVersionRequested;//版本
	WSADATA wsaData;//库版本信息结构
	SOCKADDR_IN addrSrv;//服务器地址结构
    int err;
    #endif
	
    int port = atoi(argv);
    std::cout << "init" << std::endl;
    #ifdef _WIN32
	/*初始化套接字库*/
	wVersionRequested = MAKEWORD(2, 2);//version:1.1
	err = WSAStartup(wVersionRequested, &wsaData);
	if (err != 0)
		exit(-1);
	if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2)
	{
		WSACleanup();//不符合则关闭套接字库
		exit(-2);
	}
    #else
    struct sockaddr_in addrSrv;
    #endif
	/*设置服务器地址*/
    memset(&addrSrv, 0, sizeof(addrSrv));
    #ifdef _WIN32
	addrSrv.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
    #else
    addrSrv.sin_addr.s_addr =16777343;// inet_addr("127.0.0.1");
    #endif
	addrSrv.sin_family = AF_INET;
	addrSrv.sin_port = htons(port);
	connect(sockClient, (struct sockaddr*)&addrSrv, sizeof(addrSrv));
	/*连接服务器
	if (connect(sockClient, (SOCKADDR*)&addrSrv, sizeof(SOCKADDR)) == SOCKET_ERROR)
	{
        //std::cout << WSAGetLastError() << std::endl;
        perror("connect error!");
		//printf("fail to connect！");
        exit(-3);
	}*/
    std::cout << "init finished" << std::endl;
    send(sockClient, RequestBandwidthCommand, strlen(RequestBandwidthCommand), 0);
    char recvBuf[1024]="0";
    recv(sockClient, recvBuf, sizeof(recvBuf), 0);
    std::cout << "TEST:!!!!" << recvBuf <<std::endl;
}
float cmdinfer::GetEstimatedBandwidth() {
    //以下为新增代码
    send(sockClient, RequestBandwidthCommand, strlen(RequestBandwidthCommand), 0);
    char recvBuf[1024];
    recv(sockClient, recvBuf, sizeof(recvBuf), 0);
    return atof(recvBuf);
    /*
    std::uint64_t bandwidth = 0;
    std::cout << RequestBandwidthCommand << std::endl;
    std::cin >> bandwidth;
    return static_cast<float>(bandwidth);*/
}
