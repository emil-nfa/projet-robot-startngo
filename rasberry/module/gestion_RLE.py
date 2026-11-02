"""Module de gestion et de traitement d'images au format RLE pour le tracé de chemins."""

from typing import Dict, List, Tuple, Union
import numpy as np
from PIL import Image, ImageDraw
import serial
import matplotlib.pyplot as plt

DEBUG = True


def string_to_matrix(rle_string: str) -> tuple:
    """Convertit une chaîne de caractères au format RLE en matrice binaire."""
    elements = rle_string.split(":")

    elements = [int(val) for val in elements]
    width = elements.pop(0)
    height = elements.pop(0)  # noqa: F841

    pixel_list: List[int] = []
    index = 0
    current_color = elements.pop(0)
    run_length = elements[index]

    while index < len(elements):
        if run_length == 0:
            index += 1
            current_color = (current_color + 1) % 2
            if index != len(elements):
                run_length = elements[index]
        run_length -= 1
        pixel_list.append(current_color)

    if pixel_list:
        pixel_list.pop()  # Retire le dernier élément superflu si présent

    # --- CORRECTION ROBUSTE : Ajustement et découpage propre en lignes égales ---
    target_size = width * height
    if len(pixel_list) < target_size:
        if DEBUG:
            print(f"[AVERTISSEMENT RLE] Pixel list trop courte ({len(pixel_list)} au lieu de {target_size}), remplissage avec des 0.")
        pixel_list.extend([0] * (target_size - len(pixel_list)))
    elif len(pixel_list) > target_size:
        pixel_list = pixel_list[:target_size]

    # Découpage direct par tranches de 'width' pour garantir une forme homogène
    matrix = [pixel_list[i : i + width] for i in range(0, target_size, width)]
    # --------------------------------------------------------------------------

    return (width,height),matrix

def remove_inner_fill(matrix: Union[List[List[int]], np.ndarray]) -> np.ndarray:
    """Détecte et retire les contours internes pleins pour ne garder que les lignes."""
    height = len(matrix)
    width = len(matrix[0])
    filtered_matrix = np.copy(matrix)

    for i in range(height):
        for j in range(width):
            neighbor_sum = 0
            if i > 0:
                neighbor_sum += matrix[i - 1][j]
            if i < (height - 1):
                neighbor_sum += matrix[i + 1][j]
            if j > 0:
                neighbor_sum += matrix[i][j - 1]
            if j < (width - 1):
                neighbor_sum += matrix[i][j + 1]
            if neighbor_sum == 4:
                filtered_matrix[i][j] = 0

    return filtered_matrix

def display_matrix(matrix: Union[List[List[int]], np.ndarray]) -> None:
    """Affiche graphiquement la matrice binaire sous forme d'image."""
    pixels = (1 - np.array(matrix, dtype=np.uint8)) * 255
    image = Image.fromarray(pixels)
    image.show()

def debug_visualize_path(matrix: Union[List[List[int]], np.ndarray], coordinates_path: List[Tuple[int, int, int]]) -> None:
    """Affiche graphiquement la matrice et superpose le tracé calculé pour le débogage (Rouge = Tracé, Bleu = Déplacement)."""
    # Conversion de la matrice en image RGB pour permettre l'affichage en couleur
    pixels = (1 - np.array(matrix, dtype=np.uint8)) * 255
    base_image = Image.fromarray(pixels).convert("RGB")
    draw = ImageDraw.Draw(base_image)

    if coordinates_path:
        prev_x, prev_y = coordinates_path[0][0], coordinates_path[0][1]
        for coord in coordinates_path[1:]:
            x, y, pen_state = coord
            # Inversion des axes si nécessaire selon le repère (ici x=colonne, y=ligne)
            if pen_state == 1:
                # Stylo baissé : Ligne rouge
                draw.line([(prev_y, prev_x), (y, x)], fill="red", width=1)
            else:
                # Stylo levé / saut : Ligne bleue discontinue ou fine
                draw.line([(prev_y, prev_x), (y, x)], fill="blue", width=1)
            prev_x, prev_y = x, y

    base_image.show()

def test_neighbors(
    neighbor_order: List[int],
    current_pixel: Tuple[int, int],
    matrix: Union[List[List[int]], np.ndarray],
    visited_pixels: Dict[Tuple[int, int], int],
) -> Tuple[int, Union[Tuple[int, int], int]]:
    """Teste les voisins selon l'ordre de la liste et renvoie le mouvement et le nouveau pixel."""
    movement_direction = 0
    next_pixel: Union[Tuple[int, int], int] = 0
    row_idx, col_idx = current_pixel
    width = len(matrix[0])
    height = len(matrix)
    index = 0

    while movement_direction == 0 and index < len(neighbor_order):
        direction = neighbor_order[index]
        if direction == 1 and col_idx < width - 1:
            if matrix[row_idx][col_idx + 1] == 1 and visited_pixels[(row_idx, col_idx + 1)] == 0:
                movement_direction = 1
                next_pixel = (row_idx, col_idx + 1)
        elif direction == 2 and row_idx < height - 1:
            if matrix[row_idx + 1][col_idx] == 1 and visited_pixels[(row_idx + 1, col_idx)] == 0:
                movement_direction = 2
                next_pixel = (row_idx + 1, col_idx)
        elif direction == 3 and col_idx > 0:
            if matrix[row_idx][col_idx - 1] == 1 and visited_pixels[(row_idx, col_idx - 1)] == 0:
                movement_direction = 3
                next_pixel = (row_idx, col_idx - 1)
        elif direction == 4 and row_idx > 0:
            if matrix[row_idx - 1][col_idx] == 1 and visited_pixels[(row_idx - 1, col_idx)] == 0:
                movement_direction = 4
                next_pixel = (row_idx - 1, col_idx)
        elif direction == 5 and row_idx > 0 and col_idx > 0:
            if matrix[row_idx - 1][col_idx - 1] == 1 and visited_pixels[(row_idx - 1, col_idx - 1)] == 0:
                movement_direction = 5
                next_pixel = (row_idx - 1, col_idx - 1)
        elif direction == 6 and row_idx > 0 and col_idx < width - 1:
            if matrix[row_idx - 1][col_idx + 1] == 1 and visited_pixels[(row_idx - 1, col_idx + 1)] == 0:
                movement_direction = 6
                next_pixel = (row_idx - 1, col_idx + 1)
        elif direction == 7 and row_idx < height - 1 and col_idx < width - 1:
            if matrix[row_idx + 1][col_idx + 1] == 1 and visited_pixels[(row_idx + 1, col_idx + 1)] == 0:
                movement_direction = 7
                next_pixel = (row_idx + 1, col_idx + 1)
        elif direction == 8 and row_idx < height - 1 and col_idx > 0:
            if matrix[row_idx + 1][col_idx - 1] == 1 and visited_pixels[(row_idx + 1, col_idx - 1)] == 0:
                movement_direction = 8
                next_pixel = (row_idx + 1, col_idx - 1)
        index += 1

    return movement_direction, next_pixel


def find_black_neighbor(
    current_pixel: Tuple[int, int],
    matrix: Union[List[List[int]], np.ndarray],
    visited_pixels: Dict[Tuple[int, int], int],
) -> Tuple[int, Union[Tuple[int, int], int]]:
    """Détermine les voisins valides en fonction de la position du pixel dans la matrice."""
    row_idx, col_idx = current_pixel
    height = len(matrix)
    width = len(matrix[0])

    if row_idx == 0:
        if col_idx == 0:
            return test_neighbors([1, 2, 7], current_pixel, matrix, visited_pixels)
        elif col_idx == width - 1:
            return test_neighbors([2, 3, 8], current_pixel, matrix, visited_pixels)
        else:
            return test_neighbors([1, 2, 3, 7, 8], current_pixel, matrix, visited_pixels)
    elif row_idx == height - 1:
        if col_idx == 0:
            return test_neighbors([1, 4, 6], current_pixel, matrix, visited_pixels)
        elif col_idx == width - 1:
            return test_neighbors([3, 4, 5], current_pixel, matrix, visited_pixels)
        else:
            return test_neighbors([1, 3, 4, 5, 6], current_pixel, matrix, visited_pixels)
    elif col_idx == 0:
        return test_neighbors([1, 2, 4, 6, 7], current_pixel, matrix, visited_pixels)
    elif col_idx == width - 1:
        return test_neighbors([2, 3, 4, 5, 8], current_pixel, matrix, visited_pixels)
    else:
        return test_neighbors([1, 2, 3, 4, 5, 6, 7, 8], current_pixel, matrix, visited_pixels)


def trace_path(
    matrix: Union[List[List[int]], np.ndarray],
) -> Tuple[List[int], List[Tuple[int, int]], List[Tuple[int, int]]]:
    """Calcule le trajet complet de traçage à travers la matrice."""
    height = len(matrix)
    width = len(matrix[0])
    visited_pixels: Dict[Tuple[int, int], int] = {(i, j): 0 for i in range(height) for j in range(width)}
    path: List[int] = [0]
    start_points: List[Tuple[int, int]] = []
    pixel_list: List[Tuple[int, int]] = []

    for i in range(height):
        for j in range(width):
            if matrix[i][j] == 1 and visited_pixels[(i, j)] == 0:
                start_point = (j, i)  # <-- Inversion ici : X=j (colonne), Y=i (ligne)
                current_pixel = (i, j)  # La matrice garde (row, col) pour les recherches
                visited_pixels[current_pixel] = 1
                start_points.append(start_point)
                pixel_list.append(current_pixel)

                movement_direction, next_pixel = find_black_neighbor(current_pixel, matrix, visited_pixels)
                path.append(movement_direction)

                if next_pixel != 0 and isinstance(next_pixel, tuple):
                    pixel_list.append(next_pixel)
                    visited_pixels[next_pixel] = 1

                while movement_direction != 0 and isinstance(next_pixel, tuple):
                    movement_direction, next_pixel = find_black_neighbor(next_pixel, matrix, visited_pixels)
                    path.append(movement_direction)
                    if next_pixel != 0 and isinstance(next_pixel, tuple):
                        pixel_list.append(next_pixel)
                        visited_pixels[next_pixel] = 1

    return path, start_points, pixel_list


def simplify_path(path: List[int]) -> List[Tuple[int, int]]:
    """Simplifie le trajet en regroupant les mouvements adjacents identiques (compteur, mouvement)."""
    simplified_path: List[Tuple[int, int]] = []
    counter = 1
    for i in range(len(path)):
        if i < (len(path) - 1) and path[i] == path[i + 1]:
            counter += 1
        else:
            simplified_path.append((counter, path[i]))
            counter = 1
    return simplified_path


def displacement_to_vector(movement: Tuple[int, int]) -> Tuple[int, int]:
    """Convertit un mouvement élémentaire en vecteur de coordonnées (dx, dy) [X, Y]."""
    step_count = movement[0]
    direction = movement[1]
    
    # X = horizontal (col), Y = vertical (row)
    vectors = {
        1: (step_count, 0),      # Droite (X+)
        2: (0, step_count),      # Bas (Y+) -> Ajustez à (0, -step_count) si l'axe Y est inversé
        3: (-step_count, 0),     # Gauche (X-)
        4: (0, -step_count),     # Haut (Y-) -> Ajustez à (0, step_count) si l'axe Y est inversé
        5: (-step_count, -step_count), # Haut-Gauche
        6: (step_count, -step_count),  # Haut-Droite
        7: (step_count, step_count),   # Bas-Droite
        8: (-step_count, step_count),  # Bas-Gauche
    }
    return vectors.get(direction, (0, 0))


def simplify_path_to_coordinates(
    simplified_path: List[Tuple[int, int]], start_points: List[Tuple[int, int]]
) -> List[Tuple[int, int, int]]:
    """Convertit un trajet simplifié en une liste de coordonnées absolues avec état du stylo."""
    coordinates_path: List[Tuple[int, int, int]] = []
    pen_state = 1
    remaining_start_points = start_points.copy()
    for i in range(len(simplified_path)):
        if i < len(simplified_path) - 2 and simplified_path[i + 1][1] == 0:
            pen_state = 1
            vector = displacement_to_vector(simplified_path[i])
            last_x, last_y = (coordinates_path[-1][0], coordinates_path[-1][1]) if coordinates_path else (0, 0)
            coordinates_path.append((last_x + vector[0], last_y + vector[1], pen_state))
        elif simplified_path[i][1] == 0:
            pen_state = 0
            if remaining_start_points:
                new_start = remaining_start_points[0] # Déjà au format (j, i) -> (X, Y)
                remaining_start_points = remaining_start_points[1:]
            else:
                pen_state = 1
                new_start = (0, 0)
            coordinates_path.append((new_start[0], new_start[1], pen_state))
        else:
            vector = displacement_to_vector(simplified_path[i])
            last_x, last_y = (coordinates_path[-1][0], coordinates_path[-1][1]) if coordinates_path else (0, 0)
            coordinates_path.append((last_x + vector[0], last_y + vector[1], pen_state))

    return coordinates_path

def afficher_coordonnees_gradient(coordonnees):
    """
    Affiche une liste de coordonnées (x, y) avec un gradient de couleur 
    du noir au blanc selon l'ordre des points, avec l'axe Y inversé pour correspondre à l'image.
    """
    if not coordonnees:
        print("La liste de coordonnées est vide.")
        return

    # Séparation des coordonnées X et Y
    x_vals = [pt[0] for pt in coordonnees]
    y_vals = [pt[1] for pt in coordonnees]
    
    n = len(coordonnees)
    intensites = np.linspace(0.1, 1.0, n)
    
    plt.figure(figsize=(8, 6))
    
    scatter = plt.scatter(x_vals, y_vals, c=intensites, cmap='gray', s=60, edgecolors='black', linewidth=0.5)
    
    # Optionnel : relier les points par une ligne pour voir le tracé
    plt.plot(x_vals, y_vals, color='gray', linestyle='--', alpha=0.4, zorder=0)
    
    # Ajout d'une barre de couleur indicative
    cbar = plt.colorbar(scatter)
    cbar.set_label('Progression du tracé (Noir -> Blanc)')
    
    plt.title("Affichage des coordonnées redressé (Repère Image)")
    plt.xlabel("Axe X")
    plt.ylabel("Axe Y")
    
    # --- CORRECTION : Inversion de l'axe Y pour remettre l'image à l'endroit ---
    plt.gca().invert_yaxis()
    
    plt.grid(True, linestyle=':', alpha=0.6)
    plt.show()

from PIL import Image

def image_vers_rle(chemin_image):
    """
    Convertit une image en une chaîne RLE au format :
    width:height:base_color:count1,count2,count3,...
    """
    # Ouverture de l'image et conversion en nuances de gris (0 à 255)
    # Vous pouvez aussi utiliser convert('1') pour du noir et blanc pur (0 ou 255)
    img = Image.open(chemin_image).convert('L')
    width, height = img.size
    
    # Récupération de tous les pixels ligne par ligne
    pixels = list(img.getdata())
    
    if not pixels:
        return f"{width}:{height}:0:"
    
    # La couleur de base est celle du tout premier pixel
    base_color = pixels[0]
    runs = []
    current_count = 0
    current_color = 1-base_color
    
    # Parcours de tous les pixels pour compter les répétitions alternées
    for pixel in pixels:
        if pixel == current_color:
            current_count += 1
        else:
            runs.append(str(current_count))
            current_color = pixel
            current_count = 1
            
    # Ne pas oublier d'ajouter le dernier groupe
    runs.append(str(current_count))
    
    # Construction de la chaîne finale
    rle_string = f"{width}:{height}:{base_color}:" + ":".join(runs)
    return rle_string

def gestion_rle(string_rle):
    taille,matrix = string_to_matrix(string_rle)
    matrix_cleaned = remove_inner_fill(matrix)
    movements, start_points, _ = trace_path(matrix_cleaned)
    simplified = simplify_path(movements)
    path_coords = simplify_path_to_coordinates(simplified, start_points)
    return taille,path_coords[:-1]


if __name__ == "__main__":
    string_rle = image_vers_rle("image.png")
    taille,path_coords = gestion_rle(string_rle)
    print("Coordonnées finales du chemin :", path_coords)

    
    # Lancement du débogage visuel
    # debug_visualize_path(matrix_cleaned, path_coords)
    # debug_visualize_path(matrix, [])
    afficher_coordonnees_gradient(path_coords)
    
