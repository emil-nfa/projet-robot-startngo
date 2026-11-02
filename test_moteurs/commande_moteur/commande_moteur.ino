#include <Dynamixel2Arduino.h>

#define DXL_SERIAL Serial

const uint8_t DXL_ID = 3;               // ⚠️ Essayez 1 si votre moteur est neuf ou n'a jamais été changé
const float DXL_PROTOCOL_VERSION = 2.0; // ⚠️ Mettez 1.0 si vous avez un vieux modèle (ex: AX-12A)

// Si vous utilisez un shield avec contrôle de direction matériel, -1 est correct.
Dynamixel2Arduino dxl(DXL_SERIAL, -1);

void setup() {
  // Initialisation de la communication série pour le moniteur série (si disponible)
  // Attention sur Arduino Uno/Nano, Serial est partagé avec le shield.
  // Si vous utilisez un Arduino avec SerialUSB (Due, Zero, etc.), vous pouvez utiliser SerialUSB pour le debug.
  
  // Démarrage du port Dynamixel (essayez 1000000 si 57600 ne donne rien)
  dxl.begin(57600);
  dxl.setPortProtocolVersion(DXL_PROTOCOL_VERSION);

  // Étape de diagnostic : test de présence du moteur
  if (dxl.ping(DXL_ID)) {
    tone(48,220,200);
    // Le moteur a répondu ! On peut le configurer
    dxl.torqueOff(DXL_ID);
    dxl.setOperatingMode(DXL_ID, OP_POSITION);
    dxl.torqueOn(DXL_ID);
  } else {
    tone(48,440,1000);
    // Le moteur ne répond pas. 
    // Vérifiez : 1) L'alimentation externe, 2) L'ID (testez 1), 3) Le baudrate (testez 1000000), 4) Le protocole (testez 1.0)
  }
}

void loop() {
  // Si le moteur a répondu dans le setup, il bougera ici
  dxl.setGoalPosition(DXL_ID, 0.0, UNIT_DEGREE);
  delay(1500);

  dxl.setGoalPosition(DXL_ID, 180.0, UNIT_DEGREE);
  delay(1500);
}