import matplotlib.pyplot as plt
import numpy as np

def afficher_coordonnees_gradient(coordonnees):
    """
    Affiche une liste de coordonnées (x, y) avec un gradient de couleur 
    du noir au blanc selon l'ordre des points.
    
    :param coordonnees: Liste de tuples ou listes, ex: [(x1, y1), (x2, y2), ...]
    """
    if not coordonnees:
        print("La liste de coordonnées est vide.")
        return

    # Séparation des coordonnées X et Y
    x_vals = [pt[0] for pt in coordonnees]
    y_vals = [pt[1] for pt in coordonnees]
    
    # Création d'une échelle de valeurs de 0 à 1 pour le gradient (ordre des points)
    # 0 = Noir, 1 = Blanc
    n = len(coordonnees)
    intensites = np.linspace(0.1, 1.0, n)  # On commence à 0.1 pour garder un minimum de visibilité sur fond blanc
    
    plt.figure(figsize=(8, 6))
    
    # Utilisation du colormap 'gray' (ou 'Greys')
    # Pour un fond blanc, le noir (0) et les nuances sombres ressortent bien, 
    # les points les plus clairs (1) seront proches du blanc.
    scatter = plt.scatter(x_vals, y_vals, c=intensites, cmap='gray', s=60, edgecolors='black', linewidth=0.5)
    
    # Optionnel : relier les points par une ligne pour voir le tracé
    plt.plot(x_vals, y_vals, color='gray', linestyle='--', alpha=0.4, zorder=0)
    
    # Ajout d'une barre de couleur indicative
    cbar = plt.colorbar(scatter)
    cbar.set_label('Progression du tracé (Noir -> Blanc)')
    
    plt.title("Affichage des coordonnées en gradient Noir à Blanc")
    plt.xlabel("Axe X")
    plt.ylabel("Axe Y")
    plt.grid(True, linestyle=':', alpha=0.6)
    plt.show()

# --- Exemple d'utilisation ---
if __name__ == "__main__":
    # Génération de quelques coordonnées d'exemple (ex: une spirale ou une sinusoïde)
    t = np.linspace(0, 10, 50)
    x_exemple = t * np.cos(t)
    y_exemple = t * np.sin(t)
    liste_coords = list(zip(x_exemple, y_exemple))
    
    # Appel de la fonction
    afficher_coordonnees_gradient(liste_coords)