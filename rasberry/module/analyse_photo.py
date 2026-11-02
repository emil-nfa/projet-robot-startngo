import cv2
import numpy as np
import matplotlib.pyplot as plt
np.set_printoptions(threshold=np.inf)

try : 
    from module.constante import *
except:
    from constante import *

def load_and_preprocess(image_path, max_dim=1000):
    if IMAGE_DEBUG and __name__ != "__main__":
        img=cv2.imread(IMAGE_DEBUG)
    else:
        img = cv2.imread(image_path)
    if img is None:
        raise FileNotFoundError(f"Impossible de charger l'image : {image_path}")

    height, width = img.shape[:2]
    if max(height, width) > max_dim:
        scale = max_dim / max(height, width)
        img = cv2.resize(img, (0, 0), fx=scale, fy=scale)

    gray = cv2.cvtColor(img, cv2.COLOR_BGR2GRAY)
    hsv = cv2.cvtColor(img, cv2.COLOR_BGR2HSV)
    blur = cv2.GaussianBlur(gray, (7, 7), 0)
    return img, gray, hsv, blur

def detect_circle(blur_img):
    circles = cv2.HoughCircles(
        blur_img, cv2.HOUGH_GRADIENT, dp=1.2, minDist=100,
        param1=50, param2=30, minRadius=50, maxRadius=0
    )
    if circles is None:
        return None
    circles = np.uint16(np.around(circles))
    return circles[0][0]

def detect_marked_rectangle(gray_img, circle_px):
  cx_px, cy_px, r_px = circle_px

  # 1. Détection des contours sur l'image brute (évite l'effet de bord du masque)
  blurred = cv2.GaussianBlur(gray_img, (5, 5), 0)
  edges = cv2.Canny(blurred, 50, 150)

  # 2. Création du masque circulaire
  circle_mask = np.zeros_like(gray_img)
  cv2.circle(circle_mask, (cx_px, cy_px), int(r_px * 0.95), 255, -1)

  # 3. On ne garde QUE les contours situés À L'INTÉRIEUR du cercle
  edges_inside = cv2.bitwise_and(edges, edges, mask=circle_mask)

  # (Optionnel) Fermer les petits espaces dans les traits du rectangle
  kernel = np.ones((3, 3), np.uint8)
  closed_edges = cv2.morphologyEx(edges_inside, cv2.MORPH_CLOSE, kernel)

  # 4. Recherche des contours finaux
  contours, _ = cv2.findContours(
      closed_edges, cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE
  )
  if not contours:
    return None

  # Récupération du plus grand contour (qui sera votre rectangle à l'intérieur)
  largest_contour = max(contours, key=cv2.contourArea)
  rect = cv2.minAreaRect(largest_contour)

  return rect  # Renvoie la structure minAreaRect

def convert_to_real_coordinates(rect, circle_px, diametre_reel):
    cx_px, cy_px, r_px = circle_px
    rayon_reel = diametre_reel / 2.0
    echelle = rayon_reel / r_px

    # Extraire centre, dimensions et angle du rectangle en pixels
    (cx_r_px, cy_r_px), (w_px, h_px), angle = rect

    # Dimensions réelles
    w_r = w_px * echelle
    h_r = h_px * echelle

    # Centre réel
    cx_r = (cx_r_px - cx_px) * echelle
    cy_r = (cy_px - cy_r_px) * echelle

    # Coins du rectangle orienté
    box_px = cv2.boxPoints(rect)
    coins_reels = []
    for pt in box_px:
        x_r = (pt[0] - cx_px) * echelle
        y_r = (cy_px - pt[1]) * echelle
        coins_reels.append((x_r, y_r))

    coins_reels = np.array(coins_reels)
    center = np.mean(coins_reels, axis=0)
    angles = np.arctan2(coins_reels[:, 1] - center[1], coins_reels[:, 0] - center[0])
    rect_corners_sorted = coins_reels[np.argsort(-angles)]
    
    return rect_corners_sorted, (cx_r, cy_r), (w_r, h_r), angle, rayon_reel

def verifier_taille_feuille(size_real):
    """Refuse une détection dont les dimensions ne ressemblent pas à un A4 (une photo sans feuille passe sinon)."""
    petit, grand = sorted(size_real)
    if petit <= 0 or not 1.25 <= grand / petit <= 1.55:    # A4 : 1,414
        raise Exception(f"Erreur : feuille de {petit:.1f} x {grand:.1f} cm, ce n'est pas un A4.")
    for mesure, attendu in ((petit, TAILLE_FEUILLE_LARGEUR), (grand, TAILLE_FEUILLE_LONGUEUR)):
        if abs(mesure - attendu) > TOLERANCE_FEUILLE * attendu:
            raise Exception(f"Erreur : feuille de {petit:.1f} x {grand:.1f} cm, ce n'est pas un A4.")


def get_largest_aligned_square_inside(center_real, size_real, angle_deg):
    """
    Calcule le plus grand carré aligné sur les axes (X, Y)
    strictement INCLUS à l'intérieur du rectangle orienté (feuille).
    """
    cx, cy = center_real
    w, h = size_real

    # Angle en radians
    rad = np.radians(angle_deg)
    cos_a = abs(np.cos(rad))
    sin_a = abs(np.sin(rad))

    # Les 4 droites des bords du rectangle sont à une distance w/2 et h/2 du centre.
    # Pour un carré centré en (cx, cy) de demi-côté 'half', la condition pour que 
    # les 4 coins (cx ± half, cy ± half) restent dans le rectangle orienté s'écrit :
    # half * cos_a + half * sin_a <= w / 2   ET   half * sin_a + half * cos_a <= h / 2
    
    # Demi-côté maximal autorisé par les 2 paires de bords du rectangle
    max_half_w = (w / 2.0) / (cos_a + sin_a)
    max_half_h = (h / 2.0) / (cos_a + sin_a)

    # On prend la contrainte la plus stricte
    half = min(max_half_w, max_half_h)
    square_side = half*2
    # Définition des 4 coins du carré aligné sur X, Y
    square_corners = np.array([
        [cx - half, cy + half],  # Haut-Gauche
        [cx + half, cy + half],  # Haut-Droit
        [cx + half, cy - half],  # Bas-Droit
        [cx - half, cy - half]   # Bas-Gauche
    ])

    return square_corners, square_side


    """
    Parse une chaîne RLE au format "hauteur:largeur:l0:l1:l2:l3..."
    
    :param rle_string: Chaîne de caractères RLE séparée par des ':'
    :return: (rle_data, shape) compatible avec le reste du code
    """
    tokens = [int(val) for val in rle_string.strip().split(':')]
    
    # 1. Extraire la taille
    height = tokens[0]
    width = tokens[1]
    shape = (height, width)
    
    # 2. Extraire la liste des longueurs (counts)
    rle_data = tokens[2:]
    
    return rle_data, shape

def plot_geometry(rayon_reel, rect_reels, square_reels, square_side, rle_data=None, rle_shape=None):
    """
    Affiche le graphique X, Y avec le cercle, la feuille (rectangle),
    le carré inscrit et l'image RLE plaquée dans le carré si fournie.
    """
    fig, ax = plt.subplots(figsize=(8, 8))

    # 1. Cercle
    cercle_plot = plt.Circle((0, 0), rayon_reel, color='black', fill=False, linewidth=2, label='Cercle intérieur')
    ax.add_patch(cercle_plot)

    # 2. Rectangle Orange (Feuille)
    rect_plot = np.vstack([rect_reels, rect_reels[0]])
    ax.plot(rect_plot[:, 0], rect_plot[:, 1], color='darkorange', linewidth=2, label='Feuille (Rectangle)')
    ax.fill(rect_plot[:, 0], rect_plot[:, 1], color='orange', alpha=0.3)

    # 3. Carré aligné inscrit (Bleu)
    square_plot = np.vstack([square_reels, square_reels[0]])
    ax.plot(square_plot[:, 0], square_plot[:, 1], color='blue', linewidth=2, linestyle='--', label=f'Carré intérieur ({square_side:.1f}x{square_side:.1f})')

    # 4. Affichage de l'image RLE dans le carré (si fournie)
    if rle_data is not None and rle_shape is not None:
        rle_img = decode_rle(rle_data, rle_shape)
        
        # Emprise spatiale du carré dans le repère X, Y
        # square_reels : [Haut-Gauche, Haut-Droit, Bas-Droit, Bas-Gauche]
        x_min, x_max = square_reels[0][0], square_reels[1][0]
        y_min, y_max = square_reels[2][1], square_reels[0][1]

        # Affichage de l'image matricielle dans les limites [x_min, x_max, y_min, y_max]
        ax.imshow(
            rle_img, 
            cmap='Blues', 
            alpha=0.6, 
            extent=[x_min, x_max, y_min, y_max], 
            origin='upper'
        )
    else:
        ax.fill(square_plot[:, 0], square_plot[:, 1], color='blue', alpha=0.2)

    # Points des coins du carré
    ax.scatter(square_reels[:, 0], square_reels[:, 1], color='blue', zorder=5, s=35)
    for i, (x, y) in enumerate(square_reels, 1):
        ax.annotate(f" S{i}({x:.1f}, {y:.1f})", (x, y), fontsize=9, color='darkblue', fontweight='bold')

    # Origine (0,0)
    ax.scatter([0], [0], color='black', marker='+', s=100, linewidth=2, label='Centre (0,0)')

    # Configuration du graphique
    limite = rayon_reel * 1.25
    ax.set_xlim(-limite, limite)
    ax.set_ylim(-limite, limite)
    ax.set_aspect('equal')
    ax.grid(True, linestyle='--', alpha=0.6)
    ax.set_xlabel("X (unités réelles)")
    ax.set_ylabel("Y (unités réelles)")
    ax.set_title("Carré Max Inscrit et Image RLE au Repère Cartésien")
    ax.legend(loc='upper right')

    plt.show()

# --- FONCTION PRINCIPALE ---
def analyse_photo(image_path, diametre_reel):
    """
    Traite l'image, extrait la géométrie (cercle, feuille orange, carré inscrit)
    et affiche le graphique avec l'image RLE décodée dans le carré si elle est fournie.
    """
    img, gray, hsv, blur = load_and_preprocess(image_path)
    circle_px = detect_circle(blur)

    if circle_px is None:
        raise Exception ("Erreur : Aucun cercle détecté.")

    # rect = detect_orange_rectangle(hsv, gray, circle_px)
    rect = detect_marked_rectangle(gray, circle_px)
    if rect is None:
        raise Exception ("Erreur : Feuille introuvable.")
        

    # Calculs géométriques
    rect_reels, center_real, size_real, angle_deg, rayon_reel = convert_to_real_coordinates(rect, circle_px, diametre_reel)
    verifier_taille_feuille(size_real)
    square_reels, square_side = get_largest_aligned_square_inside(center_real, size_real, angle_deg)

    
    if __name__ == "__main__":
        # Affichage des coordonnées dans la console
        print("\n==================================================")
        print(" COORD. DU CARRÉ INSCRI DANS LA FEUILLE (Origine = Centre)")
        print("==================================================")
        print(f"Taille du carré : {square_side:.2f} x {square_side:.2f}")
        for i, (x, y) in enumerate(square_reels, 1):
            print(f"Coin S{i} : X = {x:6.2f} | Y = {y:6.2f}")
        print("==================================================\n")

        # Affichage graphique avec l'image RLE plaquée dans le carré
        plot_geometry(
            rayon_reel, 
            rect_reels, 
            square_reels, 
            square_side
        )
    return square_reels

if __name__ == "__main__":
    analyse_photo("image_cercle_feuille.jpg",18.6)