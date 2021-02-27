#!/usr/bin/python3
import socket
  
UDP_PORT = 5006
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.bind(('1.2.3.16', UDP_PORT))
  
while True:
	data, addr = sock.recvfrom(1024)
	if not data:
		break
	print(data)
	#sock.sendto(data, addr)
  
sock.close()
