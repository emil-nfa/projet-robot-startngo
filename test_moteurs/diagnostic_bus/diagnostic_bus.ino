// Diagnostic du bus Dynamixel, en lecture seule : aucune commande de couple ni de position.
// Le shield occupe Serial, le rapport part donc sur Serial2 (câble TTL vers la Pi, /dev/ttyUSB0, 9600 bauds).
// Serial1 (RLE du jury) est écouté pour vérifier son câblage.

#include <DynamixelShield.h>

using namespace ControlTableItem;

DynamixelShield dxl;

const int PIN_BUZZER = 48;
const uint8_t ID_MAX = 20;
const long BAUDS[] = {57600, 1000000, 115200, 9600};
const int NB_BAUDS = 4;

// Adresses de la table de contrôle du XL330
const uint16_t ADDR_OPERATING_MODE = 11;
const uint16_t ADDR_TORQUE_ENABLE = 64;
const uint16_t ADDR_HARDWARE_ERROR = 70;
const uint16_t ADDR_PRESENT_POSITION = 132;
const uint16_t ADDR_PRESENT_VOLTAGE = 144;
const uint16_t ADDR_PRESENT_TEMPERATURE = 146;

long lire(uint8_t id, uint16_t adresse, uint16_t taille) {
  int32_t valeur = 0;
  if (dxl.read(id, adresse, taille, (uint8_t*)&valeur, sizeof(valeur), 100) != (int32_t)taille) return -1;
  return valeur;
}

void rapportMoteur(uint8_t id) {
  Serial2.print("MOTEUR id=");
  Serial2.print(id);
  Serial2.print(" modele=");
  Serial2.print(dxl.getModelNumber(id));
  Serial2.print(" mode=");
  Serial2.print(lire(id, ADDR_OPERATING_MODE, 1));
  Serial2.print(" couple=");
  Serial2.print(lire(id, ADDR_TORQUE_ENABLE, 1));
  Serial2.print(" erreur=");
  Serial2.print(lire(id, ADDR_HARDWARE_ERROR, 1));
  Serial2.print(" position=");
  Serial2.print(lire(id, ADDR_PRESENT_POSITION, 4));
  Serial2.print(" tension_dV=");
  Serial2.print(lire(id, ADDR_PRESENT_VOLTAGE, 2));
  Serial2.print(" temp_C=");
  Serial2.println(lire(id, ADDR_PRESENT_TEMPERATURE, 1));
}

void setup() {
  Serial1.begin(9600);
  Serial2.begin(9600);
  dxl.setPortProtocolVersion(2.0);
  tone(PIN_BUZZER, 440, 100);
}

void loop() {
  Serial2.println("=== BALAYAGE ===");
  int total = 0;
  for (int b = 0; b < NB_BAUDS; b++) {
    dxl.begin(BAUDS[b]);
    delay(50);
    for (uint8_t id = 0; id <= ID_MAX; id++) {
      if (dxl.ping(id)) {
        Serial2.print("baud=");
        Serial2.print(BAUDS[b]);
        Serial2.print(" ");
        rapportMoteur(id);
        total++;
      }
    }
  }
  Serial2.print("TOTAL moteurs=");
  Serial2.println(total);

  Serial2.print("SERIAL1 octets_recus=");
  Serial2.println(Serial1.available());
  while (Serial1.available()) Serial1.read();

  delay(2000);
}
