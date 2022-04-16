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
        DataPacket( int64_t get_rl_input_time_ms, float rtt_ms, float lost_per_sec,
                    uint8_t loss_rate, int64_t recv_throughput_bps,
                    float retrans_num, int64_t last_final_estimation_rate_bps, float inter_packet_delay_ms,
                    int last_encoded_rate_bps, int64_t last_pacing_rate_bps);
        ~DataPacket() = default;

        int64_t get_rl_input_time_ms_; // feedback时间
        float rtt_ms_;
        float lost_per_sec; // 每秒丢帧数
        uint8_t loss_rate_; // loss rate
        int64_t recv_throughput_bps_; 
        float retrans_num;
        int64_t last_final_estimation_rate_bps_;
        float inter_packet_delay_ms_; // delta time
        int last_encoded_rate_bps_;
        int64_t last_pacing_rate_bps_;
    };
    // 仿照delay_based_bwe构造rl_result
    // struct Result {
    //     Result();
    //     Result(bool probe, webrtc::DataRate target_bitrate);
    //     ~Result() = default;
    //     bool updated;
    //     bool probe;
    //     webrtc::DataRate target_bitrate;
    //     bool recovered_from_overuse;
    //     bool backoff_in_alr;
    // };

    struct Result {
        Result();
        Result(bool use_gcc_result, webrtc::DataRate target_bitrate);
        ~Result() = default;
        bool use_gcc_result_;
        webrtc::DataRate target_bitrate_;
    };

    bool to_recv_ai_fb_;
    DataPacket rl_packet_;
    Result rl_result;
    int RLSocketInit(SOCKET& RL_socket, std::string ip, int port);
    Result FromRLModule(SOCKET RL_socket);
    std::string Convert2Json(RLBasedBwe::DataPacket data_packet_);
    void SendToRL(RLBasedBwe::DataPacket data_packet_,SOCKET RL_socket);
    void RecvFromRL(SOCKET RL_socket);
    bool isNotEmpty(DataPacket &data_packet_);
    DataRate getResult(SOCKET RL_socket, float target_rate_float);
    // friend webrtc::DelayBasedBwe::Result toDelayBasedResult(RLBasedBwe::Result RL_result);
    char* DataConvert(RLBasedBwe::DataPacket &data_packet_, char* converted_data_);
};
}
extern SOCKET RL_Socket;
#endif