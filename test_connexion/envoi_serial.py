import time

def envoi_serial(ard,data):
        # Envoi des données
        ard.write(data.encode("utf-8"))
        # Petit délai pour laisser le temps à l'Arduino de traiter et renvoyer la réponse
        time.sleep(0.2)
