import cv2
import numpy as np
import socket
import sys
import pickle
import struct


if __name__ == '__main__':
	cap = cv2.VideoCapture(0)
	clientsocket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
	clientsocket.setsockopt(socket.SOL_SOCKET, socket.SO_BROADCAST, 1)

	while True:
		ret, frame = cap.read()
		data = pickle.dumps(frame)
		clientsocket.sendto(struct.pack("L", len(data))+data, ('<broadcast>', 5555))
