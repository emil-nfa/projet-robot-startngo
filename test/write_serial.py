import serial
import time

PORT_ARDUINO = "COM6"
VITESSE = 9600

arduino: serial.Serial = serial.Serial(PORT_ARDUINO, VITESSE, timeout=1)

# PAUSE OBLIGATOIRE : L'Arduino redémarre lors de l'ouverture du port série.
# Il faut attendre 2 secondes que le bootloader termine son initialisation.
time.sleep(2)

try:
    while True:
        data = input("Quelles données voulez-vous envoyer sur la carte ? ('q' pour quitter)\n")
        if data.lower() == 'q':
            break

        # Envoi des données
        arduino.write(data.encode("utf-8"))


finally:
    arduino.close()
    print("Port série fermé.")  
