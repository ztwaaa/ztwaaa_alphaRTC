import socket
import sys

def main(port):
    server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    server.setblocking(False)   # 将socket设置为非阻塞. 在创建socket对象后就进行该操作.
    port = int(port)
    server.bind(("127.0.0.1",port))
    server.listen(5)
    print(port)
    client_list = []

    while True:
        try:
            connection, addr = server.accept()
            client_list.append((connection, addr))
            print("connected:{}".format(addr))

        # accept原本是阻塞的, 等待connect, 设置setblocking(False)后, accept不再阻塞,
        # 它会(不断的轮询)要求必须有connect来连接, 不然就引发BlockingIOError, 所以为了在没有connect时,
        # 我们捕捉这个异常并pass掉.
        except BlockingIOError:
            pass

        for client_socket, client_addr in client_list:
            try:
                client_recv = client_socket.recv(1024)
                #data = 'hello 20'
                if client_recv:
                    print("receive:{}>>>{}".format(client_addr, client_recv))
                    #print(bytes("%s" % data, encoding="utf-8"))
                    client_socket.send(b'20')

                else:
                    client_socket.close()
                    print("downline:{}".format(client_addr))
                    client_list.remove((client_socket, client_addr))

            except (BlockingIOError, ConnectionResetError):
                pass

if __name__ == '__main__':
    port = sys.argv[1]
    main(port)