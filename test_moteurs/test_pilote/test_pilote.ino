// Test des moteurs piloté depuis la Pi sur Serial2 (/dev/ttyUSB0, 9600 bauds).
// Commandes : "S" = état des moteurs ; "M <id> <delta>" = déplacement relatif en pas, puis couple coupé.
// Le mode de chaque moteur n'est pas modifié. Couple coupé en dehors des déplacements.

#include <DynamixelShield.h>

using namespace ControlTableItem;

DynamixelShield dxl;

const int PIN_BUZZER = 48;
const uint8_t IDS[] = {1, 2, 3};
const int NB_IDS = 3;
const long DELTA_MAX = 1024;         // un quart de tour au maximum par commande
const uint32_t VITESSE_TEST = 40;    // environ 9 tr/min
const uint32_t ACCEL_TEST = 10;
const unsigned long DUREE_MAX_MS = 4000;
const int MODE_POSITION = 3;

void etat(uint8_t id) {
  Serial2.print("ETAT id=");
  Serial2.print(id);
  if (!dxl.ping(id)) {
    Serial2.println(" absent");
    return;
  }
  Serial2.print(" modele=");
  Serial2.print(dxl.getModelNumber(id));
  Serial2.print(" mode=");
  Serial2.print(dxl.readControlTableItem(OPERATING_MODE, id));
  Serial2.print(" couple=");
  Serial2.print(dxl.readControlTableItem(TORQUE_ENABLE, id));
  Serial2.print(" erreur=");
  Serial2.print(dxl.readControlTableItem(HARDWARE_ERROR_STATUS, id));
  Serial2.print(" position=");
  Serial2.print(dxl.getPresentPosition(id));
  Serial2.print(" courant=");
  Serial2.print(dxl.getPresentCurrent(id));
  Serial2.print(" tension_dV=");
  Serial2.print(dxl.readControlTableItem(PRESENT_INPUT_VOLTAGE, id));
  Serial2.print(" temp_C=");
  Serial2.println(dxl.readControlTableItem(PRESENT_TEMPERATURE, id));
}

void deplacer(uint8_t id, long delta) {
  if (!dxl.ping(id)) {
    Serial2.println("REFUS moteur absent");
    return;
  }
  if (abs(delta) > DELTA_MAX) {
    Serial2.println("REFUS delta trop grand");
    return;
  }
  long depart = dxl.getPresentPosition(id);
  long but = depart + delta;
  if (dxl.readControlTableItem(OPERATING_MODE, id) == MODE_POSITION && (but < 0 || but > 4095)) {
    Serial2.println("REFUS hors plage");
    return;
  }

  dxl.writeControlTableItem(PROFILE_VELOCITY, id, VITESSE_TEST);
  dxl.writeControlTableItem(PROFILE_ACCELERATION, id, ACCEL_TEST);
  dxl.setGoalPosition(id, depart);
  dxl.torqueOn(id);
  dxl.setGoalPosition(id, but);

  int courantMax = 0;
  long position = depart;
  unsigned long t0 = millis();
  while (millis() - t0 < DUREE_MAX_MS) {
    delay(100);
    position = dxl.getPresentPosition(id);
    int courant = abs((int)dxl.getPresentCurrent(id));
    if (courant > courantMax) courantMax = courant;
    if (abs(but - position) <= 5) break;
  }

  Serial2.print("MOUVEMENT id=");
  Serial2.print(id);
  Serial2.print(" depart=");
  Serial2.print(depart);
  Serial2.print(" but=");
  Serial2.print(but);
  Serial2.print(" arrivee=");
  Serial2.print(position);
  Serial2.print(" duree_ms=");
  Serial2.print(millis() - t0);
  Serial2.print(" courant_max=");
  Serial2.println(courantMax);

  dxl.torqueOff(id);
  dxl.writeControlTableItem(PROFILE_VELOCITY, id, 0);   // controle_moteur suppose le profil par défaut
  dxl.writeControlTableItem(PROFILE_ACCELERATION, id, 0);
  etat(id);
}

void setup() {
  Serial2.begin(9600);
  Serial2.setTimeout(200);
  dxl.begin(57600);
  dxl.setPortProtocolVersion(2.0);
  for (int i = 0; i < NB_IDS; i++) dxl.torqueOff(IDS[i]);
  tone(PIN_BUZZER, 440, 100);
  Serial2.println("PRET");
}

void loop() {
  if (!Serial2.available()) return;
  String ligne = Serial2.readStringUntil('\n');
  ligne.trim();
  if (ligne == "S") {
    for (int i = 0; i < NB_IDS; i++) etat(IDS[i]);
  } else if (ligne.startsWith("M ")) {
    int espace = ligne.indexOf(' ', 2);
    if (espace == -1) {
      Serial2.println("REFUS syntaxe");
      return;
    }
    deplacer(ligne.substring(2, espace).toInt(), ligne.substring(espace + 1).toInt());
  } else if (ligne.length() > 0) {
    Serial2.println("REFUS commande inconnue");
  }
  Serial2.println("FIN");
}
