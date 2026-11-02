import time
import serial

def lecture_serial(arduino:serial.Serial)->str:
    if arduino.in_waiting > 0:
        donnee:str = arduino.readline().decode('utf-8').rstrip()
        return donnee

def ecriture_serial(arduino:serial.Serial, donnee:str)->None:
    arduino.write((donnee + "\n").encode("utf-8"))
    time.sleep(0.2)