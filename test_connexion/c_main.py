import serial
import time
import cv2 

PORT_ARDUINO0 = "COM5"
PORT_ARDUINO1 = "COM6"
VITESSE = 9600

CAP = cv2.VideoCapture(0)

def setup_arduino():
    ard0:serial.Serial = serial.Serial(PORT_ARDUINO0, VITESSE, timeout=1)
    ard1:serial.Serial = serial.Serial(PORT_ARDUINO1, VITESSE, timeout=1)
    time.sleep(2)
    return (ard0,ard1)

def envoi_serial(ard,data):
        # Envoi des données
        ard.write(data.encode("utf-8"))

        # Petit délai pour laisser le temps à l'Arduino de traiter et renvoyer la réponse
        time.sleep(0.2)

def lecture_serial(ard):
    if ard.in_waiting > 0:
        # Lecture de la ligne reçue
        donnee:str = ard.readline().decode('utf-8').rstrip()
        return donnee

def prise_photo():
    if not CAP.isOpened():
        print("Erreur : Impossible d'accéder à la webcam.")
        return
    ret, frame = CAP.read()
    cv2.imwrite("photo.jpg", frame)

def analyse_photo(chemin_image, seuil_noir=50, pourcentage_requis=50.0):
  """Analyse si une image est au moins à 'pourcentage_requis' noire avec OpenCV.

  - chemin_image : chemin vers le fichier image
  - seuil_noir : valeur sous laquelle un pixel est considéré comme noir (0 à
  255)
  - pourcentage_requis : pourcentage minimum pour valider la condition (ex:
  50.0)
  """
  # Charger l'image en nuances de gris directement (0 = noir, 255 = blanc)
  img = cv2.imread(chemin_image, cv2.IMREAD_GRAYSCALE)

  if img is None:
    print(f"Erreur : Impossible de charger l'image '{chemin_image}'.")
    return False

  # Nombre total de pixels
  total_pixels = img.size

  # Compter les pixels dont la valeur est inférieure ou égale au seuil_noir
  # cv2.countNonZero compte les pixels > 0, on utilise donc une condition numpy
  pixels_noirs = (img <= seuil_noir).sum()

  # Calculer le pourcentage réel
  pourcentage_reel = (pixels_noirs / total_pixels) * 100

  print(f"--- Analyse OpenCV ---")
  print(f"Pixels noirs détectés : {pourcentage_reel:.2f}%")

  # Comparer avec le seuil souhaité
  if pourcentage_reel >= pourcentage_requis:
    print(f"Résultat : OUI, l'image est à plus de {pourcentage_requis}% noire.")
    return True
  else:
    print(f"Résultat : NON, l'image est à moins de {pourcentage_requis}% noire.")
    return False

def main():
    ard0,ard1 = setup_arduino()
    try:
        while True:
            envoi_serial(ard1,input("envoyer une donnée : "))
            x = lecture_serial(ard0)
            if x == "1":
                prise_photo()
                if analyse_photo("photo.jpg"):
                    envoi_serial(ard0, "2")
            else: 
                print(f"data envoyé : {x}")
    except Exception as e:
        print(e)
    finally:
        ard0.close()
        ard1.close()
        print("Fin du programme")

if __name__ == "__main__":
    main()