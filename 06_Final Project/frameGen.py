######################################
## Author: I-No Liao                ##
## Date of update: 2019/05/07       ##
## Project: Parallel Programming    ##
## - Generate video frames          ##
######################################

# general
import cv2
import csv
import os
import time
import argparse

# multiprocessing
from multiprocessing import pool
from multiprocessing.dummy import Pool as ThreadPool



#-----------------------#
#         Class         #
#-----------------------#
class video:

    # @param videoFile: str
    # @param outputPath: str
    def __init__(self, videoFile, outputPath):
        self.videoFile = videoFile
        self.outputPath = outputPath

    # @param coreNum: int
    # @return None
    # multi-core frame generation function
    def frameGenThread(self, coreNum):
        if not self.videoFile or not self.outputPath:
            print(">> Error: empty video file or output path")
            return None

        if not os.path.exists(self.outputPath):
            os.makedirs(self.outputPath)

        # image buffer
        images = []

        # simulation
        stamp1 = time.time()

        # read frames
        video = cv2.VideoCapture(self.videoFile)
        success, image = video.read()
        count = 1
        while success:
            images.append((count, image))
            success, image = video.read()
            count += 1

        # simulation
        stamp2 = time.time()

        # save frames
        pool = ThreadPool(coreNum)
        pool.map(self.frameSave, images)
        pool.close()
        pool.join()

        # simulation
        stamp3 = time.time()
        print(">> Simulation: retrieve frames time = %f" %(stamp2 - stamp1))
        print(">> Simulation: save frames time = %f" %(stamp3 - stamp2))

        # report
        print(">> Process completed")
        print(">> Total number of frames = %d" %(count))
        return None

    # @param imageNode: Tuple(imageNumb, image)
    # @return None
    def frameSave(self, imageNode):
        imageNum = imageNode[0]
        image = imageNode[1]
        cv2.imwrite(self.outputPath + '%d.jpg' %(imageNum), image) 
        return None



#-----------------------#
#     Main Function     #
#-----------------------#
def main():
    # argument processing
    coreNum = argParse()

    # directory
    videoName = './TTY_CYF_2018_Indonesia_Open_Final.mp4'
    outputPath = './frames/'

    # videoName = './example.mp4'
    # outputPath = './exampleFrames/'

    # print status
    print("------------------------------------------------")
    print("Processing video frames w/ %d core(s)" %(coreNum))

    # timer start
    startTime = time.time()
    
    # generate and save video frames
    myVideo = video(videoName, outputPath)
    myVideo.frameGenThread(coreNum)

    # timer stop
    stopTime = time.time()
    print(">> execution time = %f" %(stopTime - startTime))
    print("------------------------------------------------")



#-----------------------#
#     Sub-Functions     #
#-----------------------#
def argParse():
    parser = argparse.ArgumentParser()
    parser.add_argument('--coreNum', '-c', type = int, default = 1)
    
    args = parser.parse_args()
    coreNum = args.coreNum
    return coreNum 



#-----------------------#
#       Execution       #
#-----------------------#
if __name__ == "__main__":
    main()
