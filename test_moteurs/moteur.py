import serial
import time
import csv
import os
import matplotlib.pyplot as plt

# ============================================================
# CONFIGURATION
# ============================================================
PORT = "COM6"           # Port de l'Arduino Mega (ex: "COM3" ou "/dev/ttyACM0")
BAUDRATE = 115200       # Vitesse de communication série avec l'Arduino
FICHIER_CSV = "reponse_echelon_xl330-3.csv"
ANGLE_CIBLE_DEG = 90.0
ANGLE_INITIAL_DEG = 30.0

# ============================================================
# ACQUISITION VIA ARDUINO
# ============================================================
print(f"Connexion à l'Arduino sur {PORT}...")
ser = serial.Serial(PORT, BAUDRATE, timeout=1)
time.sleep(2)  # L'ouverture du port reset l'Arduino, attendre le boot

# Vider le buffer
ser.reset_input_buffer()

# Attendre que l'Arduino soit prêt
ligne = ""
while "PRET" not in ligne:
    if ser.in_waiting > 0:
        ligne = ser.readline().decode('utf-8', errors='ignore').strip()
        print(ligne)

input("Appuyez sur Entrée pour lancer l'échelon...")
ser.write(b'S')  # Envoie l'ordre de départ

temps = []
angles = []

print("Enregistrement en cours...")
while True:
    if ser.in_waiting > 0:
        ligne = ser.readline().decode('utf-8', errors='ignore').strip()
        
        if ligne == "FIN":
            break
        
        if ";" in ligne:
            try:
                t_str, angle_str = ligne.split(";")
                temps.append(float(t_str))
                angles.append(float(angle_str))
            except ValueError:
                pass

ser.close()

if len(temps) < 2:
    raise RuntimeError("Erreur : Pas assez de mesures reçues de l'Arduino.")

# ============================================================
# SAUVEGARDE ET TRACÉ
# ============================================================
with open(FICHIER_CSV, "w", newline="", encoding="utf-8") as fichier:
    writer = csv.writer(fichier, delimiter=";")
    writer.writerow(["temps_s", "angle_mesure_deg", "consigne_deg"])
    for t, angle in zip(temps, angles):
        writer.writerow([f"{t:.6f}", f"{angle:.4f}", f"{ANGLE_CIBLE_DEG:.4f}"])

print(f"Données sauvegardées dans : {os.path.abspath(FICHIER_CSV)}")
print(f"Nombre de mesures : {len(temps)}")

# Tracé de la courbe
plt.figure(figsize=(9, 5))
plt.plot(temps, angles, label="Angle mesuré", linewidth=2)
plt.axhline(ANGLE_CIBLE_DEG, color="red", linestyle="--", label="Consigne")
plt.axhline(ANGLE_INITIAL_DEG, color="gray", linestyle=":", label="Position initiale")

plt.xlabel("Temps (s)")
plt.ylabel("Angle (degrés)")
plt.title("Réponse à un échelon de position — XL330 via Arduino Mega")
plt.grid(True)
plt.legend()
plt.tight_layout()
plt.savefig("reponse_echelon_xl330.png", dpi=200)
plt.show()