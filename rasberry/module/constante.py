#Général
DEBUG = True


# Serial

PORT_ARDUINO = "/dev/ttyUSB0"
PORT_ARDUINO_DEV = "/dev/ttyACM0"
VITESSE = 115200
RLE_MIN_OCTETS = 7 # RLE_MIN_OCTETS dans controle_moteur.ino
MAX_POINTS = 800 # MAX_MOVEMENTS dans controle_moteur.ino


#Cam

ID_CAM = 0
NOM_PHOTO = "photo_cercle.jpg"
IMAGE_DEBUG = None # chemin d'une image de test, None = photo de la webcam

#Constante physique

DIAMETRE_CERCLE = 60
POSITION_CENTRE_CERCLE = (30,30) # Centre du cercle vu depuis l'origine du stylo, en cm : À MESURER sur le robot

TAILLE_CARRE = 25

TAILLE_FEUILLE_LONGUEUR = 29.7
TAILLE_FEUILLE_LARGEUR = 21

MARGE_DESSIN = 2
TOLERANCE_FEUILLE = 0.15 # écart accepté sur les dimensions mesurées de la feuille

# Reprise
NB_ESSAIS_PHOTO = 3
