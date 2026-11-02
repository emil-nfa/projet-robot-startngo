#Général
DEBUG = True


# Serial

PORT_ARDUINO = "/dev/ttyUSB0"
PORT_ARDUINO_DEV = "/dev/ttyACM0"
VITESSE = 9600
MAX_POINTS = 200 # MAX_MOVEMENTS dans controle_moteur.ino


#Cam

ID_CAM = 0
NOM_PHOTO = "photo_cercle.jpg"
IMAGE_DEBUG = None # chemin d'une image de test, None = photo de la webcam

#Constante physique

DIAMETRE_CERCLE = 18.6
POSITION_CENTRE_CERCLE = (15,15) # La position réel du centre du cercle par rapport au carré

TAILLE_CARRE = 25

TAILLE_FEUILLE_LONGUEUR = 29.7
TAILLE_FEUILLE_LARGEUR = 21

MARGE_DESSIN = 2
