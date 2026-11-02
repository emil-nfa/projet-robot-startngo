import serial
import tim


PORT_ARDUINO0 = "COM5"
PORT_ARDUINO1 = "COM6"
VITESSE = 9600

CAP = cv2.VideoCapture(0)

def setup_arduino():
    ard0:serial.Serial = serial.Serial(PORT_ARDUINO, VITESSE, timeout=1)
    ard1:serial.Serial = serial.Serial(PORT_ARDUINO, VITESSE, timeout=1)
    time.sleep(2)
    return (ard0,ard1)