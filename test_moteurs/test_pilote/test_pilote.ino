// Test des moteurs piloté depuis la Pi sur Serial2 (/dev/ttyUSB0, 9600 bauds).
// Commandes : "S" = état des moteurs ; "M <id> <delta>" = déplacement relatif en pas, puis couple coupé.
// "E <id>" = passe le moteur en position étendue (multi-tours), enregistré dans le moteur.
// Le mode n'est modifié que par cette commande. Couple coupé en dehors des déplacements.

#include <DynamixelShield.h>

using namespace ControlTableItem;

DynamixelShield dxl;

const int PIN_BUZZER = 48;
const uint8_t IDS[] = {1, 2, 3};
const int NB_IDS = 3;
const long DELTA_MAX = 20480;        // 5 tours au maximum par commande
const uint32_t VITESSE_TEST = 40;    // environ 9 tr/min
const uint32_t ACCEL_TEST = 10;
const long PAS_PAR_SECONDE = 625;    // VITESSE_TEST en pas par seconde
const unsigned long MARGE_MS = 4000;
const int COURANT_ARRET = 1100;      // mA, moteur bloqué proche de 1,47 A
const int NB_MESURES_ARRET = 5;      // 0,5 s au-dessus de COURANT_ARRET
const int NB_MESURES_IMMOBILE = 20;  // 2 s sans avancer
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
  long positionPrec = depart;
  int mesuresCourant = 0;
  int mesuresImmobile = 0;
  const char* fin = "duree";
  unsigned long dureeMax = abs(delta) * 1000 / PAS_PAR_SECONDE + MARGE_MS;
  unsigned long t0 = millis();
  while (millis() - t0 < dureeMax) {
    delay(100);
    position = dxl.getPresentPosition(id);
    int courant = abs((int)dxl.getPresentCurrent(id));
    if (courant > courantMax) courantMax = courant;
    if (abs(but - position) <= 5) {
      fin = "but";
      break;
    }
    mesuresCourant = courant > COURANT_ARRET ? mesuresCourant + 1 : 0;
    if (mesuresCourant >= NB_MESURES_ARRET) {
      fin = "courant";
      break;
    }
    mesuresImmobile = abs(position - positionPrec) < 2 ? mesuresImmobile + 1 : 0;
    positionPrec = position;
    if (mesuresImmobile >= NB_MESURES_IMMOBILE) {
      fin = "immobile";
      break;
    }
  }

  Serial2.print("MOUVEMENT fin=");
  Serial2.print(fin);
  Serial2.print(" id=");
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
  } else if (ligne.startsWith("E ")) {
    uint8_t id = ligne.substring(2).toInt();
    dxl.torqueOff(id);
    dxl.setOperatingMode(id, OP_EXTENDED_POSITION);
    etat(id);
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
