import serial
import cv2
import time

from module.constante import *

def setup()->tuple:
    arduino:serial.Serial = serial.Serial(PORT_ARDUINO, VITESSE, timeout=1)
    cam = cv2.VideoCapture(ID_CAM)
    return arduino,cam

def closing(arduino,cam):
    if arduino and arduino.is_open:
        arduino.close() 
    if cam and cam.isOpened():
        cam.release()