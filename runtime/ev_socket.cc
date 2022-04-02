#include "ev_socket.h"
#include "iostream"
#include "fstream"


#define BUFFER_SIZE 1024
#define MAX_CONNECTIONS 50

using namespace std;

fstream fd_log("sock_server.log", fstream::out | fstream::app);

struct ev_io *libevlist[MAX_CONNECTIONS] = {NULL};

/*data_input convert_to_para(char* buffer){
	data_input result;
	sscanf(buffer,"%f_%f_%f_%f_%f_%f",
	result.RTT,
	result.lost_per_sec,
	result.loss_rate,
	result.recv_rate,
	result.retrans_num,
	result.send_rate_last
	);
	return result;
}*/
void stdin_cb(struct ev_loop *loop, struct ev_io *w, int revents){
	char buffer[16];
	int ret = read(w->fd, buffer, sizeof(buffer));
	if(ret < 0){
		printf("error read!\n");
		fd_log << "error read!\n";
	}
	else if(ret == 0){
		printf("closed!\n");
		fd_log << "error read!\n";
	}
	else{
		if(strcmp(buffer,"qqq!\n")==0){
			fd_log << endl;
			fd_log.close();
			ev_io_stop (loop, w);
            ev_break (loop, EVBREAK_ALL);
		}
	}
}

void socket_read_callback(struct ev_loop *loop, struct ev_io *watcher, int revents) {
	char buffer[BUFFER_SIZE];
	ssize_t ret;

	if (EV_ERROR & revents) {
		printf("error event in read\n");
		fd_log << "error event in read\n";
		return;
	}
	// socket recv
	ret = recv(watcher->fd, buffer, BUFFER_SIZE, 0); // read stream to buffer

	//ai_model_input_t *recv_info = (ai_model_input_t*) buffer;
	//data_input recv_info = convert_to_para(buffer);
	if (ret < 0) {
		printf("read error\n");
		fd_log << "read error\n";
		return;
	}

	if (ret == 0) {
		printf("client disconnected.\n");
		fd_log << "client disconnected.\n";

		if (libevlist[watcher->fd] == NULL) {
			printf("the fd already freed[%d]\n", watcher->fd);
			fd_log << "the fd already freed" << watcher->fd << "\n";
		}
		else {
			close(watcher->fd);
			ev_io_stop(loop, libevlist[watcher->fd]);
			free(libevlist[watcher->fd]);
			libevlist[watcher->fd] = NULL;
		}
		return;
	}
	else {
		DataPacket* recv_info = (DataPacket*)buffer;
		//printf("receive message:%s\n", buffer);
		printf("recv buff from client!\nRTT:%f,\nlost_per_sec:%f,\nloss_rate:%d,\nrecv_rate:%f,\nretrans_num:%f\nsend_rate_last:%f\ninter_packet_delay_:%f\nlast_encoded_rate_:%d\n", 
				recv_info->RTT,
				recv_info->lost_per_sec,
				recv_info->loss_rate,
				recv_info->recv_rate,
				recv_info->retrans_num,
				recv_info->send_rate_last,
				recv_info->inter_packet_delay_,
				recv_info->last_encoded_rate_
			);	
		fd_log 	<<  "recv buff from client! \n" 
				<<  "RTT:" << recv_info->RTT << "\n"
				<<  "lost_per_sec:" << recv_info->lost_per_sec << "\n"
				<<  "loss_rate:" << recv_info->loss_rate << "\n"
				<< 	"recv_rate:" << recv_info->recv_rate << "\n"
				<<  "retrans_num:"  << recv_info->retrans_num  << "\n"
				<<  "send_rate_last"  << recv_info->send_rate_last << "\n"
				<<  "inter_packet_delay_"  << recv_info->inter_packet_delay_ << "\n"
				<<  "last_encoded_rate_"  << recv_info->last_encoded_rate_ << "\n";

		//fd_log  <<  "recv buffer" << buffer;
	}
	int bitRes = 2000;
	// socket send to client
	send(watcher->fd, "20", sizeof(bitRes), 0);
	// memset(send_buf, 0, sizeof(send_buf));
}

void socket_accept_callback(struct ev_loop *loop, struct ev_io *watcher, int revents) {

	struct sockaddr_in listen_sockaddr;
	socklen_t client_len = sizeof(listen_sockaddr);
	int fd;
	
	// ev_io watcher for client
	struct ev_io *client_watcher = (struct ev_io*) malloc(sizeof(struct ev_io));

	if (client_watcher == NULL) {
		printf("malloc error in accept_cb\n");
		fd_log << "malloc error in accept_cb\n";
		return;
	}

	if (EV_ERROR & revents) {
		printf("error event in accept\n");
		fd_log << "error event in accept\n";
		return;
	}

	// socket accept: get file description
	fd = accept(watcher->fd, (struct sockaddr*) &listen_sockaddr, &client_len);
	
	int flags = fcntl(fd, F_GETFL, 0); 
	if(flags < 0){
		printf("set non-block error,F_GETFL: %s (errno: %d)\n", strerror(errno), errno);
		fd_log << "set non-block error,F_GETFL:" << strerror(errno)<< "errno:" << errno << "\n";
		return ;
	}
	if(fcntl(fd, F_SETFL, flags| O_NONBLOCK) < 0){ 
		printf("set non-block error,O_NONBLOCK: %s (errno: %d)\n", strerror(errno), errno);
		fd_log << "set non-block error,O_NONBLOCK:" << strerror(errno)<< "errno:" << errno << "\n";
		return;
	}

	if (fd < 0) {
        printf("accept error! %s (errno: %d)\n",strerror(errno), errno);
		fd_log << "accept error!" << strerror(errno)<< "errno:" << errno << "\n";
		return;
	}
	// printf("I am: %d\n", getpid());

	printf("I am: %d\n", fd);

	// too much connections
	if (fd > MAX_CONNECTIONS) {
		printf("fd too large[%d], %s (errno: %d)\n", fd, strerror(errno), errno);
		fd_log << "fd too large" << fd << strerror(errno) << "errno:" << errno << "\n";
		close(fd);
		return;
	}

	if (libevlist[fd] != NULL) {
		printf("fd not NULL fd is [%d], %s (errno: %d)\n", fd, strerror(errno), errno);
		fd_log << "fd not NULL fd is" << fd << strerror(errno) << "errno:" << errno << "\n";
		return;
	}

	printf("client connected\n");
	fd_log << "client connected\n";

	// listen new client
	ev_io_init(client_watcher, socket_read_callback, fd, EV_READ);
	ev_io_start(loop, client_watcher);

	libevlist[fd] = client_watcher;
}



int main(int argc, char* argv[]) {
	struct ev_loop *loop = ev_default_loop(0);
	int port = atoi(argv[1]);
	/* socket start */
	int sockfd;
	struct sockaddr_in addr;
	int addr_len = sizeof(addr);

	struct ev_io *socket_watcher = (struct ev_io*)malloc(sizeof(struct ev_io));
	struct ev_io *stdin_watcher = (struct ev_io*)malloc(sizeof(struct ev_io));
	// struct ev_timer *timeout_watcher = (struct ev_timer*)malloc(sizeof(struct ev_timer));

	// socket
	sockfd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sockfd < 0) {
		printf("socket Error: %s (errno: %d)\n", strerror(errno), errno);
		fd_log << "socket Error:" << strerror(errno) << "errno:" << errno << "\n";
        return -1;
	}

	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);	
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");


	// bind
	if (bind(sockfd, (struct sockaddr*) &addr, sizeof(addr)) != 0) {
		printf("bind Error: %s (errno: %d)\n", strerror(errno), errno);
		fd_log << "bind Error:" << strerror(errno) << "errno:" << errno << "\n";
        return -1;
	}
	// listen
	if (listen(sockfd, SOMAXCONN) < 0) {
        printf("listen Error: %s (errno: %d)\n", strerror(errno), errno);
		fd_log << "listen Error:" << strerror(errno) << "errno:" << errno << "\n";
		return -1;
	}
	// set sockfd reuseful
	int opt = SO_REUSEADDR;
	if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (const char*) &opt, sizeof(opt)) != 0) {
		printf("setsockopt error in reuseaddr[%d]\n", sockfd);
		fd_log << "setsockopt error in reuseaddr" << sockfd << "\n";
		return -1;
	}
	/* socket end */
	
	ev_io_init(stdin_watcher, stdin_cb, 0, EV_READ);
	ev_io_start(loop, stdin_watcher);

	// 初始化IO事件，accept socket(sockfds)
	ev_io_init(socket_watcher, socket_accept_callback, sockfd, EV_READ);
	ev_io_start(loop, socket_watcher);

	ev_run(loop, 0);
	
	free(stdin_watcher);
	free(socket_watcher);

	return 0;
}