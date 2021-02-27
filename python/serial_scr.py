import serial
import time
import myMiniClass
# ser.read() - 1 byte
# ser.readline() - \n
# ser.read(10) - 10 bytes


baudrate=115200
ser = serial.Serial('/dev/ttyUSB0', baudrate, xonxoff=False, timeout=0.01)


def writeCmd(cmd):
	ser.write( str.encode(cmd) )
	#time.sleep(0.01)
	r = ser.read(100) # read as much as possible
	if r != b'':
		print(r)


def openPort():
	if not ser.isOpen():
		ser.open()


if __name__ == "__main__":
	openPort()
	if ser.isOpen():
		# cmd here
		writeCmd("Get DIH0")
	else:
		print('port aren\'t opened')

mcl = myMiniClass.MiniClass()
mcl.printX()
mcl.private_print()



