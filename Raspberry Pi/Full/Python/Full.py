import numpy as np
import cv2
 
cap = cv2.VideoCapture(0)
cascadeDetector = cv2.CascadeClassifier('haarCascade.xml')

if cap.isOpened() == False:
    print('Unable to open the camera')
else:
    print('Start grabbing, press a key on Live window to terminate')
    cv2.namedWindow('Live');
    cap.set(cv2.CAP_PROP_FRAME_WIDTH,320)
    cap.set(cv2.CAP_PROP_FRAME_HEIGHT,240)
    while( cap.isOpened() ):
        ret, frame = cap.read()

        if ret==False:
            print('Unable to grab from the camera')
            break
        
        gray = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)
        objects = cascadeDetector.detectMultiScale(gray, 1.3, 5)
        for (x,y,w,h) in objects:
            cv2.rectangle(frame,(x,y),(x+w,y+h),(255,0,0),2)
        
        cv2.imshow('Live', frame)
        #cv2.waitKey(0);
        key = cv2.waitKey(5)
        if key==255: key=-1 #Solve bug in 3.2.0
        if key >= 0:
            break
    print('Closing the camera')
 
cap.release()
cv2.destroyAllWindows()
print('bye bye!')
quit()
