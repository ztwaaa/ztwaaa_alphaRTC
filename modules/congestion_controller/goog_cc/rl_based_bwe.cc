#include "modules/congestion_controller/goog_cc/rl_based_bwe.h"
#include "modules/congestion_controller/goog_cc/delay_based_bwe.h"
// #include "modules/third_party/statcollect/json.hpp"
#include "rtc_base/logging.h"
#include "rtc_base/strings/json.h"

#include <iostream>
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

std::string RLBasedBwe::Convert2Json(RLBasedBwe::DataPacket data_packet_){

    Json::Value input_to_send;
    Json::StreamWriterBuilder writerBuilder;
    std::ostringstream os;

    input_to_send["get_rl_input_time_ms"] = Json::Value(data_packet_.get_rl_input_time_ms_);
    input_to_send["rtt_ms"] = Json::Value(data_packet_.rtt_ms_);
    input_to_send["last_final_estimation_rate_bps"] = Json::Value(data_packet_.last_final_estimation_rate_bps_);
    input_to_send["loss_rate"] = Json::Value(data_packet_.loss_rate_);
    input_to_send["recv_throughput_bps"] = Json::Value(data_packet_.recv_throughput_bps_);
    input_to_send["inter_packet_delay_ms"] = Json::Value(data_packet_.inter_packet_delay_ms_);
    input_to_send["last_encoded_rate_bps"] = Json::Value(data_packet_.last_encoded_rate_bps_);
    input_to_send["last_pacing_rate_bps_"] = Json::Value(data_packet_.last_pacing_rate_bps_);

    std::unique_ptr<Json::StreamWriter> jsonWriter(writerBuilder.newStreamWriter());
    jsonWriter->write(input_to_send, &os);
    std::string jsonStr = os.str();
    std::cout << "Json to send:\n" << jsonStr << std::endl;
    return jsonStr;
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
    
    std::string sendbuf = Convert2Json(data_packet_);
    send(RL_Socket, sendbuf.c_str(), sendbuf.size(), 0);
    // char msg[1024];
    // sprintf(msg,"rtt_ms_: %f last_final_estimation_rate_bps_ %lld loss_rate %d recv_throughput_bps_ %lld inter_packet_delay_ms_ %f last_encoded_rate_bps_ %d",
    //                         data_packet_.rtt_ms_,
    //                         data_packet_.last_final_estimation_rate_bps_,
    //                         data_packet_.loss_rate_, 
    //                         data_packet_.recv_throughput_bps_,
    //                         data_packet_.inter_packet_delay_ms_,
    //                         data_packet_.last_encoded_rate_bps_);
    // send(RL_socket,msg,strlen(msg),0);
    // RTC_LOG(LS_INFO) << "data to send: " << msg;
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
    :   get_rl_input_time_ms_(0),
        rtt_ms_(0),
        lost_per_sec(0),
        loss_rate_(0),
        recv_throughput_bps_(0),
        retrans_num(0),
        last_final_estimation_rate_bps_(0),
        inter_packet_delay_ms_(0),
        last_encoded_rate_bps_(0),
        last_pacing_rate_bps_(0) {}

RLBasedBwe::DataPacket::DataPacket( int64_t get_rl_input_time_ms, float rtt, float lost_per_sec,
                                    uint8_t loss_rate, int64_t recv_throughput_bps,
                                    float retrans_num, int64_t last_final_estimation_rate_bps,
                                    float inter_packet_delay,
                                    int last_encoded_rate, int64_t last_pacing_rate_bps)
    :   get_rl_input_time_ms_(get_rl_input_time_ms),
        rtt_ms_(rtt),
        lost_per_sec(lost_per_sec),
        loss_rate_(loss_rate),
        recv_throughput_bps_(recv_throughput_bps),
        retrans_num(retrans_num),
        last_final_estimation_rate_bps_(last_final_estimation_rate_bps),
        inter_packet_delay_ms_(inter_packet_delay),
        last_encoded_rate_bps_(last_encoded_rate),
        last_pacing_rate_bps_(last_pacing_rate_bps) {}

RLBasedBwe::Result::Result()
    :   use_gcc_result_(false),
        target_bitrate_(DataRate::Zero()) {}

RLBasedBwe::Result::Result(bool use_gcc_result, DataRate target_bitrate)
    :   use_gcc_result_(use_gcc_result),
        target_bitrate_(target_bitrate) {}

DataRate RLBasedBwe::getResult(SOCKET RL_socket, float target_rate_float){
    
    return webrtc::DataRate::BitsPerSec(target_rate_float);//??;
}

/**/
RLBasedBwe::Result RLBasedBwe::FromRLModule(SOCKET RL_socket){
    float target_rate_float;
    DataRate target_datarate = DataRate::Zero();
    target_rate_float = RecvFromRL(RL_socket);
    target_datarate = webrtc::DataRate::KilobitsPerSec(target_rate_float);
    return (target_rate_float < 0) ? Result() : Result(true, target_datarate);
}

char* RLBasedBwe::DataConvert(RLBasedBwe::DataPacket &data_packet_, char* converted_data_){
    int data_len = 6 * sizeof(float) + 5;
    if((isNotEmpty(data_packet_))&&(converted_data_!=NULL)){
        sprintf(converted_data_,"%f_%f_%d_%lld_%f_%lld_%f_%d",
        data_packet_.rtt_ms_,
        data_packet_.lost_per_sec,
        data_packet_.loss_rate_,
        data_packet_.recv_throughput_bps_,
        data_packet_.retrans_num,
        data_packet_.last_final_estimation_rate_bps_,
        data_packet_.inter_packet_delay_ms_,
        data_packet_.last_encoded_rate_bps_
        );
    }
    else {
        memset(converted_data_,0,sizeof(data_len));
    }
    return converted_data_;
}
bool RLBasedBwe::isNotEmpty(DataPacket &data_packet_){
    if( data_packet_.loss_rate_ == 0 &&
        data_packet_.lost_per_sec == 0 &&
        data_packet_.recv_throughput_bps_ == 0 &&
        data_packet_.retrans_num == 0 &&
        data_packet_.last_final_estimation_rate_bps_ == 0 &&
        data_packet_.rtt_ms_ ==0 && 
        data_packet_.inter_packet_delay_ms_ == 0 &&
        data_packet_.last_encoded_rate_bps_ == 0
    )
        return false;
    else 
        return true;
}
// webrtc::DelayBasedBwe::Result toDelayBasedResult(RLBasedBwe::Result RL_result){
//     webrtc::DelayBasedBwe::Result converted_result_ = 
//                             webrtc::DelayBasedBwe::Result(RL_result.probe, RL_result.target_bitrate);
//     return converted_result_;
// }
}
