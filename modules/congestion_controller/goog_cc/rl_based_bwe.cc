#include "modules/congestion_controller/goog_cc/rl_based_bwe.h"
#include "modules/congestion_controller/goog_cc/delay_based_bwe.h"

#ifdef _WIN32
#include <windows.h>
#include <winsock2.h>
#pragma comment(lib,"ws2_32.lib")
#else
#include<linux/socket.h>
#endif

/*RL socket*/
SOCKET RL_Socket;

namespace webrtc{
RLBasedBwe::RLBasedBwe():rl_packet_(),rl_result(){}
int RLBasedBwe::RLSocketInit(SOCKET& RL_socket){
    struct sockaddr_in server_in; 
    WORD socket_version;
    WSADATA wsadata; 
    int ret;
    unsigned long ul=1;
    socket_version = MAKEWORD(2,2);
    if(WSAStartup(socket_version, &wsadata) != 0)
    {
        printf("WSAStartup error!");
        system("pause");
        return 0;
    }

    RL_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(RL_socket == INVALID_SOCKET)
    {
        printf("invalid socket !");
        system("pause");
        return 1;
    }
    ret = ioctlsocket(RL_socket,FIONBIO,(unsigned long *)&ul);
    if(ret == SOCKET_ERROR){
        printf("setting error!!");
        exit(1);
    }
    server_in.sin_family = AF_INET;    //IPV4协议族
    server_in.sin_port = htons(5000);  //服务器的端口号
    server_in.sin_addr.S_un.S_addr = inet_addr("127.0.0.1"); //服务IP
    if(connect(RL_socket, (struct sockaddr *)&server_in, sizeof(server_in)) == SOCKET_ERROR)
    {
        printf("connect error\n");
        system("pause");
        return 1;
    }else{}

    printf("connect %s:%d\n", inet_ntoa(server_in.sin_addr), server_in.sin_port);
    return 0;
}

/*socket 模块可以在之前的模块上进行修改得来 为了展示不再详写*/
void RLBasedBwe::SendToRL(RLBasedBwe::DataPacket data_packet_,SOCKET RL_socket){
    //socket
    int data_len = 6 * sizeof(float) + 5;
    char* converted_data_ = (char*)malloc(data_len);
    converted_data_ = DataConvert(data_packet_, converted_data_);
    send(RL_socket,converted_data_,strlen(converted_data_),0);
    free(converted_data_);
    //exception  handle
    
    /*if(ret > 0)
    {
        return 
    }
    else if(ret == 0)
    {
        //当ret == 0 说明服务器掉线。
        printf("lost connection , Ip = %s\n", inet_ntoa(server_in.sin_addr));
        exit(1);
    }
    else//ret < 0
    {
        //当ret < 0 说明出现了异常 例如阻塞状态解除，或者读取数据时出现指针错误等。
        //所以我们这里要主动断开和客户端的链接。
        printf("something wrong of %s\n", inet_ntoa(server_in.sin_addr));
        closesocket(socket_client);
        exit(1);  
    }*/
}


float RLBasedBwe::RecvFromRL(SOCKET RL_socket){
    //get target_rate from RL
    char recData[255];
    int ret,err;
    float target_rate_float;
    ret = recv(RL_socket, recData, 255, 0);
    if(ret == SOCKET_ERROR){
        err = WSAGetLastError();
        if(err==WSAEWOULDBLOCK){
            return 0;
        }
        else if(err == WSAETIMEDOUT){
            printf("Timeout!!");
            exit(1);
        }
        else if(err == WSAENETDOWN){
            printf("Connection lost!!");
            exit(1);
        }
        else{
            printf("Something error!!");
            exit(1);
        }
    }
    else{
        target_rate_float = std::atof(recData);
        return target_rate_float;
    }
}
RLBasedBwe::DataPacket::DataPacket()
    :   RTT(0),
        lost_per_sec(0),
        loss_rate(0),
        recv_rate(0),
        retrans_num(0),
        send_rate_last(0) {}

RLBasedBwe::DataPacket::DataPacket( float RTT,float lost_per_sec,
                                    float loss_rate,float recv_rate,
                                    float retrans_num,float send_rate_last)
    :   RTT(RTT),
        lost_per_sec(lost_per_sec),
        loss_rate(loss_rate),
        recv_rate(recv_rate),
        retrans_num(retrans_num),
        send_rate_last(send_rate_last) {}

RLBasedBwe::Result::Result()
    :   updated(false),
        probe(false),
        target_bitrate(DataRate::Zero()),
        recovered_from_overuse(false),
        backoff_in_alr(false) {}

RLBasedBwe::Result::Result(bool probe, DataRate target_bitrate)
    :   updated(true),
        probe(probe),
        target_bitrate(target_bitrate),
        recovered_from_overuse(false),
        backoff_in_alr(false) {}

DataRate RLBasedBwe::getResult(SOCKET RL_socket, float target_rate_float){
    
    return webrtc::DataRate::BitsPerSec(target_rate_float);//??;
}

/**/
RLBasedBwe::Result RLBasedBwe::FromRLModule(SOCKET RL_socket){
    float target_rate_float;
    DataRate target_datarate = DataRate::Zero();
    target_rate_float = RecvFromRL(RL_socket);
    target_datarate = webrtc::DataRate::BitsPerSec(target_rate_float);
    return (target_rate_float==0)?Result():Result(true, target_datarate);
}

char* RLBasedBwe::DataConvert(RLBasedBwe::DataPacket &data_packet_, char* converted_data_){
    int data_len = 6 * sizeof(float) + 5;
    if((isNotEmpty(data_packet_))&&(converted_data_!=NULL)){
        sprintf(converted_data_,"%f_%f_%f_%f_%f_%f",
        data_packet_.RTT,
        data_packet_.lost_per_sec,
        data_packet_.loss_rate,
        data_packet_.recv_rate,
        data_packet_.retrans_num,
        data_packet_.send_rate_last
        );
    }
    else {
        memset(converted_data_,0,sizeof(data_len));
    }
    return converted_data_;
}
bool RLBasedBwe::isNotEmpty(DataPacket &data_packet_){
    if( data_packet_.loss_rate == 0 &&
        data_packet_.lost_per_sec == 0 &&
        data_packet_.recv_rate == 0 &&
        data_packet_.retrans_num == 0 &&
        data_packet_.send_rate_last == 0 &&
        data_packet_.RTT ==0
    )
        return false;
    else 
        return true;
}
webrtc::DelayBasedBwe::Result toDelayBasedResult(RLBasedBwe::Result RL_result){
    webrtc::DelayBasedBwe::Result converted_result_ = 
                            webrtc::DelayBasedBwe::Result(RL_result.probe, RL_result.target_bitrate);
    return converted_result_;
}
}
