# import the necessary packages
from threading import Thread
import cv2

class WebcamVideoStream:
    def __init__(self, src=0,resolution=None,framerate=30):
        # initialize the variable used to indicate if the thread should
        # be stopped
        self.stopped = False
        self.src = src
        self.resolution = list(resolution)
        self.framerate = framerate
        
    def start(self):
        # initialize the video camera stream and read the first frame
        # from the stream
        self.stream = cv2.VideoCapture(self.src)
        
        self.stream.set(cv2.CAP_PROP_FPS, self.framerate)
        if(self.resolution != None):
            self.stream.set(cv2.CAP_PROP_FRAME_WIDTH, int(self.resolution[0]))
            self.stream.set(cv2.CAP_PROP_FRAME_HEIGHT, int(self.resolution[1]))

            
        self.resolution[0] = self.stream.get(cv2.CAP_PROP_FRAME_WIDTH)
        self.resolution[1] = self.stream.get(cv2.CAP_PROP_FRAME_HEIGHT)
        
        (self.grabbed, self.frame) = self.stream.read()
        # start the thread to read frames from the video stream
        
        self.__thread = Thread(target=self.update, args=())
        self.__thread.daemon = True
        self.__thread.start()
        return self
 
    def update(self):
        # keep looping infinitely until the thread is stopped
        while True:
            # if the thread indicator variable is set, stop the thread
            if self.stopped:
                break
 
            # otherwise, read the next frame from the stream
            (self.grabbed, self.frame) = self.stream.read()
        self.stream.release()
        
    def read(self):
        # return the frame most recently read
        return self.frame
 
    def stop(self):
        # indicate that the thread should be stopped
        self.stopped = True
        self.__thread.join()
