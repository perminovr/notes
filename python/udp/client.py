#!/usr/bin/python3
import socket
from time import sleep
  
UDP_PORT = 5005
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.connect(('1.2.3.111', UDP_PORT))

while True:
	sock.send(b'Hello, World!')
	#data = sock.recv(1024)
	#if not data:
	#	break
	#print(b'echo from server: ' + data)
	sleep(1)

#iptables -t nat -A OUTPUT -s 1.2.3.4 -p udp --dport 5005 -j DNAT --to-destination 1.2.3.16:5006
