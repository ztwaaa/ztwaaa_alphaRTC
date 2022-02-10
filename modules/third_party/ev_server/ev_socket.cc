#include "ev_socket.h"
#include "iostream"
#include "fstream"

#define PORT 5000
#define BUFFER_SIZE 1024
#define MAX_CONNECTIONS 50

using namespace std;
 
fstream fd_log("sock_server.log", fstream::out | fstream::app);

struct ev_io *libevlist[MAX_CONNECTIONS] = {NULL};

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
	
	ai_model_input_t *recv_info = (ai_model_input_t*) buffer;

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
		// printf("receive message:%s\n", buffer);
		printf("recv buff from client!\nreceived_bit_rate:%d,\nframeloss:%d,\nrtt:%d,\npacket_loss_rate:%d,\nnack_sent_count:%d\n", 
				recv_info->received_bit_rate,
				recv_info->frameloss,
				recv_info->rtt,
				recv_info->packet_loss_rate,
				recv_info->nack_sent_count
			);	
		fd_log 	<<  "recv buff from client! \n" 
				<<  "received_bit_rate:" << recv_info->received_bit_rate << "\n"
				<<  "frameloss:" << recv_info->frameloss << "\n"
				<<  "rtt:" << recv_info->rtt << "\n"
				<< 	"packet_loss_rate:" << recv_info->packet_loss_rate << "\n"
				<<  "nack_sent_count:"  << recv_info->nack_sent_count  << "\n";
	}
	int bitRes = 1000000;
	// socket send to client
	send(watcher->fd, (char*)&bitRes, sizeof(bitRes), 0);
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
 

 
int main() {
	struct ev_loop *loop = ev_default_loop(0);
 
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
	addr.sin_port = htons(PORT);
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
 
