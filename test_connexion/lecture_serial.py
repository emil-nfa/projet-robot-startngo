def lecture_donnees(ard):
    if arduino.in_waiting > 0:
        # Lecture de la ligne reçue
        donnee:str = arduino.readline().decode('utf-8').rstrip()
        return list(map(int, donnee.split(",")))