import cv2

# Démarrer la webcam (0 = caméra par défaut)
cap = cv2.VideoCapture(1)

if not cap.isOpened():
    print("Erreur : Impossible d'accéder à la webcam.")
    exit()

print("Appuyez sur ESPACE pour prendre la photo, ou sur 'q' pour quitter.")

while True:
    # Lire l'image image par image
    ret, frame = cap.read()
    
    if not ret:
        print("Erreur de lecture du flux vidéo.")
        break

    # Afficher le flux vidéo en direct
    cv2.imshow("Webcam - Espace pour capturer", frame)

    key = cv2.waitKey(1) & 0xFF

    # Touche ESPACE pour sauvegarder l'image
    if key == ord(' '):
        cv2.imwrite("photo.jpg", frame)
        print("Photo enregistrée sous 'photo.jpg' !")
        break

    # Touche 'q' pour quitter sans enregistrer
    elif key == ord('q'):
        print("Fermeture sans prise de photo.")
        break

# Libérer la caméra et fermer les fenêtres
cap.release()
cv2.destroyAllWindows()