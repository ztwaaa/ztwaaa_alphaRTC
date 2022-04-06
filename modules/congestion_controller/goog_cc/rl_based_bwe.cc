#include "modules/congestion_controller/goog_cc/rl_based_bwe.h"
#include "modules/congestion_controller/goog_cc/delay_based_bwe.h"
#include "modules/third_party/statcollect/json.hpp"
#include "rtc_base/logging.h"
#include<cstring>
#ifdef _WIN32
#include<windows.h>
#include<winsock2.h>
#pragma comment(lib,"ws2_32.lib")
#else
#include<linux/socket.h>
#endif
/*RL socket*/
SOCKET RL_Socket;

namespace webrtc{
RLBasedBwe::RLBasedBwe():rl_packet_(),rl_result(){}
int RLBasedBwe::RLSocketInit(SOCKET& RL_socket,int port){
    struct sockaddr_in server_in; 
    WORD socket_version;
    WSADATA wsadata; 
    int ret;
    unsigned long ul=1;
    socket_version = MAKEWORD(2,2);
    if(WSAStartup(socket_version, &wsadata) != 0)
    {
        //printf("WSAStartup error!");
        RTC_LOG(LS_INFO) << "socket WSAStartup error";
        system("pause");
        return 0;
    }

    RL_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(RL_socket == INVALID_SOCKET)
    {
        //printf("invalid socket !");
        RTC_LOG(LS_INFO) << "invalid socket !";
        system("pause");
        return 1;
    }
    ret = ioctlsocket(RL_socket,FIONBIO,(unsigned long *)&ul);
    if(ret == SOCKET_ERROR){
        //printf("setting error!!");
        RTC_LOG(LS_INFO) << "socket setting error!!";
        exit(1);
    }
    server_in.sin_family = AF_INET;    //IPV4协议族
    server_in.sin_port = htons(port);  //服务器的端口号
    server_in.sin_addr.S_un.S_addr = inet_addr("127.0.0.1"); //服务IP
    while(connect(RL_socket, (struct sockaddr *)&server_in, sizeof(server_in)) == SOCKET_ERROR)
    {
        //printf("connect error\n");
        RTC_LOG(LS_INFO) << "socket connect error\n";
        int err=WSAGetLastError();
        RTC_LOG(LS_INFO) << "socket-err-code: " << err;
        if(err!=10035)
            break;
        //system("pause");
        //return 1;
    }

    //printf("connect %s:%d\n", inet_ntoa(server_in.sin_addr), server_in.sin_port);
    RTC_LOG(LS_INFO) << "socket connect "<< inet_ntoa(server_in.sin_addr) <<":"<< server_in.sin_port;
    return 0;
}

/*socket 模块可以在之前的模块上进行修改得来 为了展示不再详写*/
void RLBasedBwe::SendToRL(RLBasedBwe::DataPacket data_packet_,SOCKET RL_socket){
    //socket
    /*int data_len = 6 * sizeof(float) + 5;
    char* converted_data_ = (char*)malloc(data_len+1);
    converted_data_ = DataConvert(data_packet_, converted_data_);
    RTC_LOG(LS_INFO) << "data to send:" << converted_data_;
    RTC_LOG(LS_INFO) << "data length:" << strlen(converted_data_);
    send(RL_socket,converted_data_,1024,0);
    int err=WSAGetLastError();
    RTC_LOG(LS_INFO) << "socket-err-code" << err;
    free(converted_data_);
    converted_data_=NULL;*/
    /*send(RL_socket,(char*)&data_packet_,sizeof(data_packet_),0);
    RTC_LOG(LS_INFO) << "data to send:" << (char*)&data_packet_;
    int err=WSAGetLastError();
    RTC_LOG(LS_INFO) << "socket-err-code" << err;*/
    char msg[1024];
    sprintf(msg,"RTT: %f send_rate_last %f loss_rate %d recv_rate %lld inter_packet_delay_ %f last_encoded_rate_ %d",
                            data_packet_.RTT,
                            data_packet_.send_rate_last,
                            data_packet_.loss_rate, 
                            data_packet_.recv_rate * 1000,
                            data_packet_.inter_packet_delay_,
                            data_packet_.last_encoded_rate_);
    send(RL_socket,msg,strlen(msg),0);
    RTC_LOG(LS_INFO) << "data to send: " << msg;
    int err = WSAGetLastError();
    RTC_LOG(LS_INFO) << "socket-err-code: " << err;
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
    char recData[10]="";
    int ret,err;
    ret = recv(RL_socket, recData, 9, 0);
    if(ret == SOCKET_ERROR){
        err = WSAGetLastError();
        if(err==WSAEWOULDBLOCK){
            return 0;
        }
        else if(err == WSAETIMEDOUT){
            //printf("Timeout!!");
            RTC_LOG(LS_INFO) << "socket Timeout!!";
            exit(1);
        }
        else if(err == WSAENETDOWN){
            //printf("Connection lost!!");
            RTC_LOG(LS_INFO) << "socket Connection lost!!";
            exit(1);
        }
        else{
            //printf("Something error!!");
            RTC_LOG(LS_INFO) << "socket Something error!!";
            exit(1);
        }
    }
    else{
        //float target_rate_float=0;
        //sprintf(recData,"hello %f",target_rate_float);
        RTC_LOG(LS_INFO) << "recv data:" << recData;
        return 0;
    }
}
RLBasedBwe::DataPacket::DataPacket()
    :   RTT(0),
        lost_per_sec(0),
        loss_rate(0),
        recv_rate(0),
        retrans_num(0),
        send_rate_last(0),
        inter_packet_delay_(0),
        last_encoded_rate_(0) {}

RLBasedBwe::DataPacket::DataPacket( float RTT, float lost_per_sec,
                                    uint8_t loss_rate, int64_t recv_rate,
                                    float retrans_num,float send_rate_last,
                                    float inter_packet_delay,
                                    int last_encoded_rate)
    :   RTT(RTT),
        lost_per_sec(lost_per_sec),
        loss_rate(loss_rate),
        recv_rate(recv_rate),
        retrans_num(retrans_num),
        send_rate_last(send_rate_last),
        inter_packet_delay_(inter_packet_delay),
        last_encoded_rate_(last_encoded_rate) {}

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
    target_datarate = webrtc::DataRate::KilobitsPerSec(target_rate_float);
    return (target_rate_float==0)?Result():Result(true, target_datarate);
}

char* RLBasedBwe::DataConvert(RLBasedBwe::DataPacket &data_packet_, char* converted_data_){
    int data_len = 6 * sizeof(float) + 5;
    if((isNotEmpty(data_packet_))&&(converted_data_!=NULL)){
        sprintf(converted_data_,"%f_%f_%d_%lld_%f_%f_%f_%d",
        data_packet_.RTT,
        data_packet_.lost_per_sec,
        data_packet_.loss_rate,
        data_packet_.recv_rate * 1000,
        data_packet_.retrans_num,
        data_packet_.send_rate_last,
        data_packet_.inter_packet_delay_,
        data_packet_.last_encoded_rate_
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
        data_packet_.RTT ==0 && 
        data_packet_.inter_packet_delay_ == 0 &&
        data_packet_.last_encoded_rate_ == 0
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
