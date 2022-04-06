#ifndef MODULES_CONGESTION_CONTROLLER_GOOG_CC_RL_BASED_BWE_H_
#define MODULES_CONGESTION_CONTROLLER_GOOG_CC_RL_BASED_BWE_H_

#include <stddef.h>
#include <stdint.h>
#include <memory>
#include <vector>
#include "RL_based_bwe.h"
#include "api/units/data_rate.h"
#include "modules/congestion_controller/goog_cc/delay_based_bwe.h"
#ifdef _WIN32
#include<windows.h>
#include<winsock2.h>
#pragma comment(lib,"ws2_32.lib")
#else
#include<linux/socket.h>
#endif

namespace webrtc{
class RLBasedBwe{
public:
    RLBasedBwe();
    struct DataPacket{
        DataPacket();
        DataPacket( float RTT, float lost_per_sec,
                    uint8_t loss_rate, int64_t recv_rate,
                    float retrans_num, float send_rate_last, float inter_packet_delay_,
                    int last_encoded_rate_);
        ~DataPacket() = default;
        float RTT;
        float lost_per_sec; // lost frames
        uint8_t loss_rate; // loss rate
        int64_t recv_rate;
        float retrans_num;
        float send_rate_last;
        float inter_packet_delay_;
        int last_encoded_rate_;
    };
    struct Result {
        Result();
        Result(bool probe, webrtc::DataRate target_bitrate);
        ~Result() = default;
        bool updated;
        bool probe;
        webrtc::DataRate target_bitrate;
        bool recovered_from_overuse;
        bool backoff_in_alr;
    };
    DataPacket rl_packet_;
    Result rl_result;
    int RLSocketInit(SOCKET& RL_socket,int port);
    Result FromRLModule(SOCKET RL_socket);
    void SendToRL(RLBasedBwe::DataPacket data_packet_,SOCKET RL_socket);
    float RecvFromRL(SOCKET RL_socket);
    bool isNotEmpty(DataPacket &data_packet_);
    DataRate getResult(SOCKET RL_socket, float target_rate_float);
    friend webrtc::DelayBasedBwe::Result toDelayBasedResult(RLBasedBwe::Result RL_result);
    char* DataConvert(RLBasedBwe::DataPacket &data_packet_, char* converted_data_);
};
}
extern SOCKET RL_Socket;
#endif