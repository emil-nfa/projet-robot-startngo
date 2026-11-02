from module.constante import *

from module.setup import setup, closing
from module.gestion_serial import *
from module.gestion_photo import prise_photo
from module.gestion_RLE import gestion_rle
from module.analyse_photo import analyse_photo
from module.gestion_coordonee import coords_mvmt_vers_cercle, coords_cercle_vers_reel, encoder_coords


def log(*strings):
  # On convertit chaque argument en chaîne (str) pour éviter l'erreur avec .join()
  string = " ".join(map(str, strings))
  print((string + "\n") * DEBUG, end="")


def main():
    log("[main]", "lancement du programme")
    arduino, cam = setup()
    running = True
    
    log("[main]","setup completed")
    string_rle = None

    try : 
        while running:
            if not string_rle:
                try:
                    string_rle = lecture_serial(arduino)
                    if string_rle:
                        log("[lecture serial]" , string_rle)
                    else:
                        continue
                except Exception as e:
                    log("[lecture serial]", str(e))
            else:
                try:
                    prise_photo(cam,NOM_PHOTO)
                    log("[prise photo]", "photo prise")
                except Exception as e:
                    log("[prise photo]", str(e))
                    closing(arduino, cam)
                    arduino, cam = setup()
                    continue
                
                try:
                    taille,coords_mvmt = gestion_rle(string_rle)
                    log("[analyse RLE]" , str(taille) , str(coords_mvmt))
                except Exception as e:
                    log("[analyse RLE]", str(e))
                    string_rle = None
                    continue

                try:
                    coords_carre_cercle = analyse_photo(NOM_PHOTO, DIAMETRE_CERCLE)
                    log("[analyse photo]" , str(coords_carre_cercle))
                except Exception as e:
                    log("[analyse photo]" , str(e))
                    prise_photo(cam, NOM_PHOTO)
                    continue

                try: 
                    coords_mvmt_cercle = coords_mvmt_vers_cercle(taille, coords_mvmt, coords_carre_cercle, MARGE_DESSIN)
                    log("[gestion coords]" , str(coords_mvmt_cercle))
                    coords_mvmt_reel = coords_cercle_vers_reel(coords_mvmt_cercle, POSITION_CENTRE_CERCLE)
                    log("[gestion coords]" , str(coords_mvmt_reel))
                except Exception as e:
                    log("[gestion coords]" , str(e))
                    prise_photo(cam, NOM_PHOTO)
                    continue

                try:
                    log("[ecriture serial] Début !")
                    if len(coords_mvmt_reel) > MAX_POINTS:
                        log("[ecriture serial] ATTENTION :", len(coords_mvmt_reel), "points, la Mega n'en garde que", MAX_POINTS)
                    string_serial = encoder_coords(coords_mvmt_reel)
                    log("[ecriture serial] [encodage]", string_serial)
                    ecriture_serial(arduino, string_serial)
                    log("[ecriture serial]" , "Liste des coordonnés de mouvement envoyé avec succés")
                except Exception as e:
                    log("[ecriture serial]" , str(e))
                    closing(arduino, cam)
                    arduino, cam = setup()
                    continue

                string_rle = None
                log("[main] programme exectuté avec succés, en attente d'une nouvelle string RLE")
    except KeyboardInterrupt:
        log("[main]", "Fin du programme !")
    except Exception as e:
        log("[main]", str(e))
    finally:
        closing(arduino,cam)

if __name__ == "__main__":
    main()
