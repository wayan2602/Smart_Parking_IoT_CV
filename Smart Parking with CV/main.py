import cv2
import numpy as np
import os
import cvzone
import pickle

import paho.mqtt.client as mqtt
from random import randrange, uniform
import time

mqttBroker = "mqtt.eclipseprojects.io"
client = mqtt.Client("Temperature_Outside")
client.connect(mqttBroker)

width, height = 107, 48
cap = cv2.VideoCapture('carPark.mp4')

with open('CarParkPos', 'rb') as f:
    posisition_list = pickle.load(f)


def CheckParkSpace(imgPro):
    space_count = 0
    for pos in posisition_list:
        x, y = pos
        imgCrop = imgPro[y:y+height, x:x+width]
        # cv2.imshow(str(x*y), imgCrop)
        count = cv2.countNonZero(imgCrop)
        # cvzone.putTextRect(img, str(count), (x, y+height-5),
        #                   scale=1.2, thickness=2, offset=0)
        if count < 950:
            cv2.rectangle(img, pos, (pos[0] + width,
                                     pos[1] + height), (0, 255, 0), 2)
            cvzone.putTextRect(img, 'available', (x, y+height-3),
                               scale=1.2, thickness=2, offset=0, colorR=(0, 255, 0), colorT=(255, 255, 255))
            space_count += 1
        else:
            cv2.rectangle(img, pos, (pos[0] + width,
                                     pos[1] + height), (0, 0, 255), 2)
            cvzone.putTextRect(img, 'not avail', (x, y+height-3),
                               scale=1.2, thickness=2, offset=0, colorR=(0, 0, 255), colorT=(255, 255, 255))

    cvzone.putTextRect(img, 'available space ' + str(space_count), (50, 60),
                       scale=3, thickness=3, offset=5, colorR=(0, 255, 0), colorT=(255, 255, 255))

    # while (space_count != 0):
    randNumber = randrange(10)
    client.publish("space_count", space_count)
    print("Just published " + str(space_count) + " to Topic space_count")
    # time.sleep(1)


while True:
    if cap.get(cv2.CAP_PROP_POS_FRAMES) == cap.get(cv2.CAP_PROP_FRAME_COUNT):
        cap.set(cv2.CAP_PROP_POS_FRAMES, 0)
    success, img = cap.read()

    imgGray = cv2.cvtColor(img, cv2.COLOR_BGR2GRAY)
    imgBlur = cv2.GaussianBlur(imgGray, (3, 3), 1)

    imgTh = cv2.adaptiveThreshold(
        imgBlur, 255, cv2.ADAPTIVE_THRESH_GAUSSIAN_C, cv2.THRESH_BINARY_INV, 25, 16)
    imgMed = cv2.medianBlur(imgTh, 5)
    kernel = np.ones((3, 3), np.uint8)
    imgDialate = cv2.dilate(imgMed, kernel, iterations=1)
    CheckParkSpace(imgDialate)

    # for pos in posisition_list:
    # cv2.rectangle(img, pos, (pos[0] + width,
    #  pos[1] + height), (255, 0, 255), 2)

    cv2.imshow('Image', img)
    # cv2.imshow('ImageBlur', imgDialate)
    cv2.waitKey(10)
