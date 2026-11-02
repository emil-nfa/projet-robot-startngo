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


def calculer_coordonnees(cam, string_rle):
    """De l'image RLE à la chaîne envoyée à la Mega. La photo est reprise si la feuille n'est pas trouvée."""
    taille, coords_mvmt = gestion_rle(string_rle)
    log("[analyse RLE]", taille, len(coords_mvmt), "points")

    derniere_erreur = None
    for essai in range(1, NB_ESSAIS_PHOTO + 1):
        try:
            prise_photo(cam, NOM_PHOTO)
            coords_carre_cercle = analyse_photo(NOM_PHOTO, DIAMETRE_CERCLE)
            log("[analyse photo]", coords_carre_cercle.tolist())
            break
        except Exception as e:
            derniere_erreur = e
            log("[analyse photo] essai", essai, str(e))
    else:
        raise derniere_erreur

    coords_mvmt_cercle = coords_mvmt_vers_cercle(taille, coords_mvmt, coords_carre_cercle, MARGE_DESSIN)
    coords_mvmt_reel = coords_cercle_vers_reel(coords_mvmt_cercle, POSITION_CENTRE_CERCLE)
    if len(coords_mvmt_reel) > MAX_POINTS:
        log("[gestion coords] ATTENTION :", len(coords_mvmt_reel), "points, la Mega n'en garde que", MAX_POINTS)
    return encoder_coords(coords_mvmt_reel)


def main():
    log("[main]", "lancement du programme")
    arduino, cam = setup()
    log("[main]", "setup completed")

    try:
        while True:
            string_rle = lecture_serial(arduino)
            if len(string_rle) < RLE_MIN_OCTETS:
                continue  # parasite, la Mega ne répond rien non plus
            log("[lecture serial]", string_rle)
            try:
                string_serial = calculer_coordonnees(cam, string_rle)
                log("[ecriture serial]", string_serial)
                ecriture_serial(arduino, string_serial)
                log("[main] coordonnees envoyees, en attente d'une nouvelle string RLE")
            except Exception as e:
                log("[main] image abandonnee :", str(e))
                ecriture_serial(arduino, "")  # ligne vide : la Mega reprend tout de suite
    except KeyboardInterrupt:
        log("[main]", "Fin du programme !")
    finally:
        closing(arduino, cam)


if __name__ == "__main__":
    main()
