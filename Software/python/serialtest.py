#!/usr/bin/env python

import serial
import sys

#  TODO: serial port detecten statt immer acm0

port = '/dev/ttyACM0'

if (len(sys.argv) == 2):
	port = sys.argv[1]

ser = serial.Serial(port, 9600, timeout=1)

ser.write('MAGIC_PING');
ser.flush()
while True:
	if (ser.inWaiting() > 0):
		print ser.read(),
	

			



