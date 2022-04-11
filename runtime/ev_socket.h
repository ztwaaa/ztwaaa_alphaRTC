#ifndef __ev_socket_h_
#define __ev_socket_h_

#include <ev.h>
#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <stdint.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <ev.h>

struct DataPacket{
        DataPacket();
        DataPacket( float RTT,float lost_per_sec,
                    uint8_t loss_rate,float recv_rate_bps_,
                    float retrans_num,float send_rate_last, float inter_packet_delay,
                    int last_encoded_rate);
        ~DataPacket() = default;
        float RTT;
        float lost_per_sec;
        uint8_t loss_rate;
        float recv_rate_bps_;
        float retrans_num;
        float send_rate_last;
        float inter_packet_delay_ms_;
        int last_encoded_rate_bps_;
    };
DataPacket::DataPacket()
    :   RTT(0),
        lost_per_sec(0),
        loss_rate(0),
        recv_rate_bps_(0),
        retrans_num(0),
        send_rate_last(0),
        inter_packet_delay_ms_(0),
        last_encoded_rate_bps_(0) {}

DataPacket::DataPacket( float RTT,float lost_per_sec,
                                    uint8_t loss_rate,float recv_rate_bps_,
                                    float retrans_num,float send_rate_last,
                                    float inter_packet_delay,
                                    int last_encoded_rate)
    :   RTT(RTT),
        lost_per_sec(lost_per_sec),
        loss_rate(loss_rate),
        recv_rate_bps_(recv_rate_bps_),
        retrans_num(retrans_num),
        send_rate_last(send_rate_last),
        inter_packet_delay_ms_(inter_packet_delay),
        last_encoded_rate_bps_(last_encoded_rate) {}
#ifdef __cplusplus
extern "C" {
#endif
/*typedef struct ai_params
{
	uint32_t		received_bit_rate;
	int				frameloss;
	uint32_t		rtt;
	uint8_t			packet_loss_rate;
	int				nack_sent_count;
}ai_model_input_t;*/
#ifdef __cplusplus
}
#endif
#endif // !__epoll_sender_h_
