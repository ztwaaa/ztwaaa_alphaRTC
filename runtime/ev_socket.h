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
                    uint8_t loss_rate,float recv_rate,
                    float retrans_num,float send_rate_last, float inter_packet_delay_);
        ~DataPacket() = default;
        float RTT;
        float lost_per_sec;
        uint8_t loss_rate;
        float recv_rate;
        float retrans_num;
        float send_rate_last;
        float inter_packet_delay_;
    };
DataPacket::DataPacket()
    :   RTT(0),
        lost_per_sec(0),
        loss_rate(0),
        recv_rate(0),
        retrans_num(0),
        send_rate_last(0),
        inter_packet_delay_(0) {}

DataPacket::DataPacket( float RTT,float lost_per_sec,
                                    uint8_t loss_rate,float recv_rate,
                                    float retrans_num,float send_rate_last,
                                    float inter_packet_delay_)
    :   RTT(RTT),
        lost_per_sec(lost_per_sec),
        loss_rate(loss_rate),
        recv_rate(recv_rate),
        retrans_num(retrans_num),
        send_rate_last(send_rate_last),
        inter_packet_delay_(inter_packet_delay_) {}
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
