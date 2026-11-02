from module.constante import *

def coords_mvmt_vers_cercle(taille_rectangle_rle: tuple, coords_mvmt: list, coords_carre_cercle: list, marge: float = 0.0) -> list:
    """Prend les coordonnées des mouvements dans le rectangle RLE (origine haut-gauche)
    et les traduit dans le carré défini par rapport au centre du cercle, avec une marge (padding).

    Args:
        taille_rectangle_rle (tuple): (largeur, hauteur) du rectangle source.
        coords_mvmt (list): Liste de coordonnées sous forme de tuples ou listes [(x, y), ...].
        coords_carre_cercle (list): Limites du carré par rapport au centre du cercle [xmin, ymin, xmax, ymax].
        marge (float): Valeur du padding pour rétrécir le carré de tous les côtés.

    Returns:
        list: Les coordonnées traduites dans le repère du cercle.
    """
    w_src, h_src = taille_rectangle_rle
    
    # Extraction des limites du carré relatives au centre du cercle
    xmin, ymin, xmax, ymax = coords_carre_cercle
    
    # Application de la MARGE (rétrécissement du carré)
    xmin_m = xmin + marge
    xmax_m = xmax - marge
    ymin_m = ymin + marge
    ymax_m = ymax - marge
    
    # Dimensions de la zone utile après application de la marge
    w_dest = xmax_m - xmin_m
    h_dest = ymax_m - ymin_m
    
    coords_traduit = []
    for pt in coords_mvmt:
        x_src, y_src = pt[0], pt[1]
        
        # Normalisation des coordonnées dans le rectangle source (entre 0 et 1)
        norm_x = x_src / w_src if w_src > 0 else 0
        norm_y = y_src / h_src if h_src > 0 else 0
        
        # Projection dans le carré réduit du cercle
        x_dest = xmin_m + (norm_x * w_dest)
        y_dest = ymin_m + (norm_y * h_dest)
        
        coords_traduit.append((x_dest, y_dest,pt[2]))
        
    return coords_traduit

def coords_cercle_vers_reel(coords_cercle:list, position_cercle:tuple):
    coords_traduit = []
    for coords in coords_cercle:
        coords_traduit.append((coords[0]+position_cercle[0],-coords[1]+position_cercle[1],coords[2]))
    return(coords_traduit)

def encoder_coords(list_coords)->str:
    liste_propre=[0]
    dernier_etat = 1
    for coords in list_coords:
        liste_propre.append(f"{coords[0]},{coords[1]}")
        if coords[2] != dernier_etat:
            dernier_etat = 1 - dernier_etat
            list_coords.append(1-dernier_etat) 

    return ":".join(liste_propre)
