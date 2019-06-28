######################################
## Author: I-No Liao                ##
## Date of update: 2019/05/07       ##
## Project: Parallel Programming    ##
## - Generate ground truth          ##
######################################

# general
import csv
import os 
import time
import argparse
import numpy
from PIL import Image

# multiprocessing
import multiprocessing



#-----------------------#
#         Class         #
#-----------------------#
class heatmap:
    
    # @param width: int
    # @param height: int
    # @param size: int
    # @param intputFile: str
    # @param outputPath: str
    # @param coreNum: int
    def __init__(self, width, height, size, inputFile, outputPath, coreNum):
        self.width = width
        self.height = height
        self.size = size
        self.inputFile = inputFile
        self.outputPath = outputPath
        self.coreNum = coreNum
        self.gKernel = self.gaussianKernelInit(10)

    # @return None
    # multi-core heatmap generation (main process)
    def heatmapGen(self):
        if not self.inputFile or not self.outputPath:
            print(">> Error: empty input file or output path")
            return None

        if not os.path.exists(self.outputPath):
            os.makedirs(self.outputPath)
            
        # retrieve coordinates
        with open(self.inputFile, 'r') as csvFile:
            csvObject = csv.reader(csvFile, delimiter = ',', quotechar = '|')
            next(csvObject, None) # skip header
            dataset = [data for data in csvObject]

        # generate heatmap by multi-core processing
        cores = []
        for i in range(self.coreNum):
            cores.append(multiprocessing.Process(target = self.heatmapGenThread, args = (dataset, i, self.coreNum)))
            cores[i].start()

        for i in range(self.coreNum):
            cores[i].join()

        return None
    
    # @param dataset: List[List[str]]
    # @param coreId: int
    # @param coreNum: int
    # @return None
    # multi-core heatmap generation (thread function)
    def heatmapGenThread(self, dataset, coreId, coreNum):

        # load assignment
        N = len(dataset)
        if N % coreNum == 0:
            portion = N // coreNum
        else:
            portion = N // coreNum + 1
        start = portion * coreId
        end = portion * (coreId + 1)

        # generate heatmap
        for i in range(start, end):
            if i == N:
                break

            row = dataset[i]
            visibility = int(float(row[1]))
            fileName = row[0]

            # heatmap initialization
            heatmap = Image.new("RGB", (self.width, self.height))
            pix = heatmap.load()
            for i in range(self.width):
                for j in range(self.height):
                    pix[i,j] = (0,0,0)

            # if visible
            if visibility != 0:
                x, y = int(float(row[2])), int(float(row[3]))
                for i in range(-self.size, self.size + 1):
                    for j in range(-self.size, self.size + 1):
                        if 0 <= x + i < self.width and 0 <= y + j < self.height:
                            temp = self.gKernel[i + self.size][j + self.size]
                            if temp > 0:
                                pix[x + i,y + j] = (temp, temp, temp)

            # save heatmap
            heatmap.save(self.outputPath + "/" + fileName.split('.')[-1] + ".png", "PNG")

        return None

    # @param variance: int
    # @return List[List[float]]
    def gaussianKernelGen(self, variance):
        x, y = numpy.mgrid[-self.size:self.size + 1, -self.size:self.size + 1]
        gKernel = numpy.exp(-(x**2 + y**2)/float(2 * variance))
        return gKernel 

    # @param variance: int
    # @return List[List[int]]
    def gaussianKernelInit(self, variance):
        gKernel = self.gaussianKernelGen(variance)
        gKernel = gKernel * 255 // gKernel[len(gKernel) // 2][len(gKernel) // 2]
        gKernel = gKernel.astype(int)
        return gKernel



#-----------------------#
#     Main Function     #
#-----------------------#
def main():
    # argument processing
    coreNum = argParse()

    # Directory
    inputFile = './Badminton_label.csv'
    outputPath = "./groundTruth/"
    # inputFile = './test_200.csv'
    # outputPath = "./test_200_groundTruth/"

    if not os.path.exists(outputPath):
        os.makedirs(outputPath)

    # print status
    print("------------------------------------------------")
    print("Processing video frames w/ %d core(s)" %(coreNum))

    # timer start
    startTime = time.time()

    # generate and save heatmaps
    myHeatmap = heatmap(1280, 720, 20, inputFile, outputPath, coreNum)
    myHeatmap.heatmapGen()

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
