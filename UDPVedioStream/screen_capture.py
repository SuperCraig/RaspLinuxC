import numpy as np
import cv2
#for windows, mac users
#from PIL import ImageGrab
#for linux users
import pyscreenshot as ImageGrab
import time

#four character code object for video writer
#fourcc = cv2.VideoWriter_fourcc(*'XVID')
fourcc = cv2.VideoWriter_fourcc(*'XVID')
#video writer object
out = cv2.VideoWriter("output.avi", fourcc, 30.0, (256,128))

while True:
	#capture computer screen
	img = ImageGrab.grab(bbox=(0,0,256,256))
	#Convert imageto numpy array
	img_np = np.array(img)
	#convert color space from BGR to RGB
	frame = cv2.cvtColor(img_np, cv2.COLOR_BGR2RGB)
	#show image on opencv frame
	#cv2.imshow("Screen", frame)
	start_time = time.time()
	cv2.imwrite("img.png",frame)
	print("--- %s seconds ---"% (time.time()-start_time))
	#write frame to video writer
	out.write(frame)

	if cv2.waitKey(1) & 0xFF == ord('q'):
		break



out.release()
cv2.destroyAllWindows()
