import numpy as np
import cv2

cap = cv2.VideoCapture("/media/pi/CRAIGCHEN/WildCreatures.wmv")
fourcc = cv2.cv.CV_FOURCC(*'XVID')
videoWriter = cv2.VideoWriter("/media/pi/CRAIGCHEN/wild.avi", fourcc, 60.0, (314,314))

while True:
	#Capture frame-by-frame
	ret, frame = cap.read()

	#Our operations on the frame come here
	gray = cv2.cvtColor(frame,cv2.COLOR_BGR2RGB)

	#Display the resulting frame
	cv2.imshow('frame', gray)

	videoWriter.write(gray)

	if cv2.waitKey(1) & 0xFF == ord('q'):
		break

#When everyting done, release the capture
cap.release()
videoWriter.release()
cv2.destroyAllWindows()
