#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import sys
import json
import glob
import socket

RequestBandwidthCommand = "RequestBandwidth"


def fetch_stats(line: str)->dict:
    line = line.strip()
    try:
        stats = json.loads(line)
        return stats
    except json.decoder.JSONDecodeError:
        return None


def request_estimated_bandwidth(line: str)->bool:
    line = line.strip()
    if RequestBandwidthCommand == line:
        return True
    return False


def find_estimator_class():
    import BandwidthEstimator
    return BandwidthEstimator.Estimator

'''
def main(ifd = sys.stdin, ofd = sys.stdout):
    estimator_class = find_estimator_class()
    estimator = estimator_class()
    while True:
        line = ifd.readline()
        if not line:
            break
        if isinstance(line, bytes):# 判断line是否是byte类型
            line = line.decode("utf-8",'ignore')
        stats = fetch_stats(line)
        if stats:
            estimator.report_states(stats)
            continue
        request = request_estimated_bandwidth(line)
        if request:
            bandwidth = estimator.get_estimated_bandwidth()
            ofd.write("{}\n".format(int(bandwidth)).encode("utf-8"))
            ofd.flush()
            continue
        sys.stdout.write(line)
        sys.stdout.flush()


def main():
    estimator_class = find_estimator_class()
    estimator = estimator_class()
    server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    server.bind(("localhost", 8888))
    server.listen(0)
    connection, address = server.accept()
    while True:
        recv_str = connection.recv(4096)
        if not recv_str:
            break
        if isinstance(recv_str, bytes):
            recv_str = recv_str.decode("utf-8")
        stats = fetch_stats(recv_str)
        if stats:
            estimator.report_states(stats)
            continue
        request = request_estimated_bandwidth(recv_str)
        if request:
            bandwidth = estimator.get_estimated_bandwidth()
            connection.send(bytes("%s" % bandwidth, encoding="utf-8"))
'''
def main(port, ifd = sys.stdin, ofd = sys.stdout):
    print('python')
    estimator_class = find_estimator_class()
    estimator = estimator_class()
    port = int(port)
    server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    #server.bind(("localhost",port))
    server.bind(("127.0.0.1",port))
    server.listen(1)
    connection, address = server.accept()
    print("python init finished")
    print(port)
    while True:
        recv_str = connection.recv(1024)
        #print(recv_str)
        if not recv_str:
            break
        if isinstance(recv_str, bytes):
            recv_str = recv_str.decode("utf-8")
        #print(recv_str)
        if request_estimated_bandwidth(recv_str) is False:
            if RequestBandwidthCommand in recv_str:    
                recv_str=recv_str.replace(RequestBandwidthCommand,'')
        #print(recv_str)
        stats = fetch_stats(recv_str)
        if stats:
            estimator.report_states(stats)
            continue
        #print(recv_str)
        request = request_estimated_bandwidth(recv_str)
        if request:
            print('yes')
            bandwidth = estimator.get_estimated_bandwidth()
            connection.send(bytes("%s" % bandwidth, encoding="utf-8"))
            #continue
        else:
            continue

        #sys.stdout.write(recv_str)
        #sys.stdout.flush()
if __name__ == '__main__':
    port = sys.argv[1]
    main(port)

