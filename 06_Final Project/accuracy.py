######################################
## Author: I-No Liao                ##
## Date of update: 2019/05/07       ##
## Project: Parallel Programming    ##
## - Accuracy metrics analysis      ##
######################################

# general
import cv2
import csv
import time
import argparse

# multiprocessing
import multiprocessing



#-----------------------#
#         Class         #
#-----------------------#
class accuracy:

    # @param labelFile: str
    # @param imagePath: str
    # @param testingFile: str
    # @param predictedImagePath: str
    # @param coreNum: int
    def __init__(self, labelFile, imagePath, testingFile, predictedImagePath, coreNum):
        self.labelFile = labelFile
        self.imagePath = imagePath
        self.testingFile = testingFile
        self.predictedImagePath = predictedImagePath
        self.coreNum = coreNum
        self.groundTruth = {}
        self.groundTruthInit()

    # @return None
    # accuracy metric analysis
    def analyze(self):
        manager = multiprocessing.Manager()
        sharedMem = manager.dict()
        cores = []

        # retrieve predicted heatmap
        with open(self.testingFile, 'r') as csvFile:
            csvObject = csv.reader(csvFile, delimiter = ',', quotechar = '|')
            next(csvObject, None) # skip header
            dataset = [data for data in csvObject]

        for i in range(self.coreNum):
            cores.append(multiprocessing.Process(target = self.analyzeThread, args = (dataset, i, self.coreNum, sharedMem)))
            cores[i].start()

        for i in range(self.coreNum):
            cores[i].join()

        # accumulate numbers from all cores
        truePositive = [0, 0, 0, 0]
        falsePositive = [0, 0, 0, 0]
        negative = [0, 0, 0, 0]

        for coreId in range(self.coreNum):
            for i in range(4):
                truePositive[i] += sharedMem[coreId][0][i]
                falsePositive[i] += sharedMem[coreId][1][i]
                negative[i] += sharedMem[coreId][2][i]

        # accuracy metrics report
        precision = sum(truePositive) / (sum(truePositive) + sum(falsePositive))
        recall = sum(truePositive) / (sum(truePositive) + sum(negative[1:]) + sum(falsePositive[1:]))
        f1 = 2 * precision * recall / (precision + recall)

        print(">> ----- Performance Summary -----")
        print(">> Number of images =", (sum(truePositive) + sum(falsePositive) + sum(negative)))
        print(">> True Positive [0, 1, 2, 3] = [%d, %d, %d, %d]" %(truePositive[0], truePositive[1], truePositive[2], truePositive[3]))
        print(">> False Positive [0, 1, 2, 3] = [%d, %d, %d, %d]" %(falsePositive[0], falsePositive[1], falsePositive[2], falsePositive[3]))
        print(">> Negative [0, 1, 2, 3] = [%d, %d, %d, %d]" %(negative[0], negative[1], negative[2], negative[3]))
        print(">> Precision =", precision)
        print(">> Recall =", recall)
        print(">> F1-measure =", f1)

        return None

    # @param dataset: List[List[str]]
    # @param coreId: int
    # @param coreNum: int
    # @param sharedMem: multiprocessing.Manager().dict()
    # @return None
    def analyzeThread(self, dataset, coreId, coreNum, sharedMem):

        # load assignment
        N = len(dataset)
        if N % coreNum == 0:
            portion = N // coreNum
        else:
            portion = N // coreNum + 1
        start = portion * coreId
        end = portion * (coreId + 1)

        # initialization
        truePositive = [0, 0, 0, 0]
        falsePositive = [0, 0, 0, 0]
        negative = [0, 0, 0, 0]

        for i in range(start, end):
            if i == N:
                break

            row = dataset[i]
            imageId = row[0]

            # acquire predicted heatmap
            heatmap = cv2.imread(imageId.replace(self.imagePath, self.predictedImagePath), 0)
            if heatmap is None:
                continue

            # convert heatmap to binary image
            ret, heatmap = cv2.threshold(heatmap, 127, 255, cv2.THRESH_BINARY)
            circles = cv2.HoughCircles(heatmap, cv2.HOUGH_GRADIENT, dp = 1, minDist = 10, param2 = 6, minRadius = 3, maxRadius = 12)

            # scenario 1: groudn truth: ball is not visible
            if self.groundTruth[imageId][1] == -1 and self.groundTruth[imageId][2] == -1:
                if circles is not None:
                    falsePositive[self.groundTruth[imageId][0]] += 1
                else:
                    negative[self.groundTruth[imageId][0]] += 1

            # scenario 2: ground truth: ball is visible
            else:
                if circles is not None:
                    if len(circles[0]) == 1:
                        x = int(circles[0][0][0])
                        y = int(circles[0][0][1])
                        dx2 = pow(self.groundTruth[imageId][1] - x, 2)
                        dy2 = pow(self.groundTruth[imageId][2] - y, 2)

                        # check distance
                        if pow(dx2 + dy2, 0.5) > 7.5: 
                            falsePositive[self.groundTruth[imageId][0]] += 1
                        else:
                            truePositive[self.groundTruth[imageId][0]] += 1
                    else:
                        negative[self.groundTruth[imageId][0]] += 1
                else:
                    negative[self.groundTruth[imageId][0]] += 1

        sharedMem[coreId] = [truePositive, falsePositive, negative]
        return None

    # @return None
    # acquire ground truth coordinates from label file
    def groundTruthInit(self):
        with open(self.labelFile, 'r') as csvFile:
            csvObject = csv.reader(csvFile, delimiter = ',', quotechar = '|')
            next(csvObject, None) # skip header
            for row in csvObject:
                fileName = row[0] + '.jpg'
                if row[1] != '0':
                    visibility = int(float(row[1]))
                    x = int(row[2])
                    y = int(row[3])
                    self.groundTruth[self.imagePath + fileName] = [visibility, x, y]
                else:
                    self.groundTruth[self.imagePath + fileName] = [0, -1, -1]

        return None



#-----------------------#
#     Main Function     #
#-----------------------#
def main():
    # argument processing
    coreNum = argParse()

    # initialization
    labelFile = './Badminton_label.csv'
    imagePath = './video_frames_new/'
    testingFile = './testing_model2_new.csv'
    predictedImagePath = './model_II_new_testset_epoch_500/'

    # print status
    print("------------------------------------------------")
    print("Processing video frames w/ %d core(s)" %(coreNum))

    # timer start
    startTime = time.time()
    
    # accuracy analysis
    myAnalyzer = accuracy(labelFile, imagePath, testingFile, predictedImagePath, coreNum)
    myAnalyzer.analyze()

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
