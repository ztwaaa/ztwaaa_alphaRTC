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


#ifdef __cplusplus
extern "C" {
#endif
typedef struct ai_params
{
	uint32_t		received_bit_rate;
	int				frameloss;
	uint32_t		rtt;
	uint8_t			packet_loss_rate;
	int				nack_sent_count;
}ai_model_input_t;

#ifdef __cplusplus
}
#endif

#endif // !__epoll_sender_h_
