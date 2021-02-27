#!/usr/bin/python3

import socket

HOST = '127.0.0.1'
PORT = 6001

s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

s.connect((HOST, PORT))
s.sendall(b'Hello, world')
data = s.recv(1024)

print('Received', repr(data))
