import cv2
import matplotlib.pyplot as plt
import numpy as np

NB_IMAGES_A_JETER = 5

def prise_photo(cam,nom_photo)->None:
    if not cam.isOpened():
        raise SystemError ("Erreur : Impossible d'accéder à la webcam.")
    else:
        for _ in range(NB_IMAGES_A_JETER): # images gardées en mémoire depuis l'ouverture de la caméra
            cam.grab()
        ret, frame = cam.read()
        if not ret:
            raise SystemError ("Erreur : Impossible de lire une image de la webcam.")
        if not cv2.imwrite(nom_photo, frame):
            raise SystemError ("Erreur : Impossible d'enregistrer la photo.")

def analyse_photo_test(seuil_noir=50, pourcentage_requis=50.0)->bool:
    # Charger l'image en nuances de gris directement (0 = noir, 255 = blanc)
    img = cv2.imread(NOM_PHOTO, cv2.IMREAD_GRAYSCALE)

    if img is None:
        raise Exception (f"Erreur : Impossible de charger l'image '{chemin_image}'.")
    total_pixels = img.size
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
