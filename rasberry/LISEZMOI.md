# Robot dessinateur : code complet (équipe D)

Version du 02/11/2026, branche `fix/chaine-complete` de https://github.com/emil-nfa/projet-robot-startngo

## Déroulé

1. Le jury envoie une image RLE à la Mega par le câble TTL sur `Serial1`.
2. La Mega renvoie la chaîne à la Pi sur `Serial2` (`/dev/ttyUSB0`, 115200 bauds).
3. La Pi transforme l'image en traits (`gestion_RLE.py`).
4. La Pi prend une photo et cherche le cercle et la feuille (`gestion_photo.py`, `analyse_photo.py`), avec 3 essais au maximum.
5. La Pi place le dessin dans un carré de la feuille et l'exprime dans le repère du robot (`gestion_coordonee.py`).
6. La Pi envoie `etat:x,y:etat:x,y...` en cm. `etat` vaut 1 si le stylo est baissé pendant le trajet vers ce point.
7. La Mega attend le bouton s'il est activé, dessine, relève le stylo et revient à l'origine.

La commande des moteurs est dans `controle_moteur/controle_moteur.ino`. La Pi la téléverse avec `lancement_arduino.py`.

## Mise en route

Depuis `rasberry/` sur la Pi :

1. `pip install -r requirement.txt` si une bibliothèque manque. `arduino-cli` a besoin du core `arduino:avr` et des bibliothèques `DynamixelShield` et `Dynamixel2Arduino` (déjà installés sur la Pi le 02/11).
2. Interrupteur du shield sur `UPLOAD`, puis `python lancement_arduino.py`.
3. Interrupteur sur `DYNAMIXEL`. Placer le stylo à son point de départ, puis alimenter les moteurs.
4. `python main.py`, puis envoyer l'image sur `Serial1`.

## À vérifier sur le robot (rien de cela n'a été testé)

| Quoi | Où | Comment |
|---|---|---|
| ID des moteurs | `ID_1`, `ID_2`, `ID_STYLO` dans le `.ino` | Valeurs supposées : M288 ID 2 en bas à gauche, M288 ID 1 en bas à droite, M077 ID 3 au stylo. `test_moteurs/test_pilote` affiche les moteurs présents |
| Sens des axes | `SENS_1`, `SENS_2` dans le `.ino` | Demander un petit déplacement en x puis en y |
| Échelle | `R` dans le `.ino` | Demander 10 cm et mesurer à la règle |
| Stylo | `STYLO_HAUT`, `STYLO_BAS` dans le `.ino` | Le moteur du stylo est en position absolue. Valeurs non réglées : commencer sans stylo |
| Vitesse du lien jury | `BAUD_RLE` dans le `.ino` | 115200 d'après le programme `rle` du cours, choisi par l'équipe ; l'ancien code utilisait 9600. À confirmer avec le câble du jury |
| Bouton | `UTILISER_BOUTON`, `PIN_BOUTON` dans le `.ino` | Désactivé tant qu'il n'est pas câblé, entre la broche 22 et GND. Exigé d'après les notes du cours d'informatique |
| Origine | `POSITION_CENTRE_CERCLE` dans `constante.py` | Le stylo démarre en dehors de la zone. Mesurer le centre du cercle depuis ce point, en cm. `(30,30)` est une valeur provisoire |
| Détection | `analyse_photo.py` | Jamais essayée avec la caméra au-dessus du robot : faire un essai avec une vraie photo |
| Orientation | `coords_cercle_vers_reel` | Vérifier que le dessin n'est ni en miroir ni tourné par rapport à la caméra |
| Résolution | `setup.py` | La Pi capture en 640 × 480 par défaut. Vérifier que le cercle entier est visible |
| Vitesse | `V_DESSIN`, `V_VIDE` dans le `.ino` | 2 et 3,5 cm/s, à essayer avec les M288. Si un moteur sature, le dessin se décale |
| Déformation | `analyse_photo.py` | Objectif grand angle, aucune correction : comparer la position dessinée à la feuille |

## Limites connues

- 800 points au maximum : au-delà, la Mega ignore la suite et la Pi l'écrit dans ses logs.
- La RAM de la Mega n'a pas été mesurée sur cette version : lire le résumé de `arduino-cli compile`.
- Si la Pi échoue, elle renvoie une ligne vide et l'image est abandonnée.
- Les bords colorés de la feuille ne sont pas utilisés : la feuille est cherchée par ses contours, avec un contrôle de taille et de ratio A4.
- `test_moteurs/test_pilote` reste à 9600 bauds sur `Serial2` (commandes `S`, `M`, `D`, `E`), contrairement à `controle_moteur`.
- Si la Pi perd le lien USB, `main.py` s'arrête : le relancer.
- Sans fin de course, l'origine est la position du stylo à la mise sous tension.
- La détection renvoie une erreur si la feuille mesurée s'écarte de plus de 25 % d'un A4.

## Tests déjà faits

- Simulation sur Mac du vrai code de la Pi et de la Mega, avec images de 64 × 64 et cas d'erreur. Les moteurs, la photo et les ports série y sont simulés.
- Un audit indépendant du code à la lecture a été fait le 02/11, sans exécution.
- La version finale du `.ino` n'a pas encore été compilée pour la Mega ni lancée sur le robot.
