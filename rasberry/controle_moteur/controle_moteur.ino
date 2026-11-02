// Robot dessinateur CoreXY : reçoit des images RLE en continu sur Serial1, les renvoie sur Serial2,
// puis reçoit et dessine les coordonnées correspondantes depuis Serial2.
// Commande haute (jacobienne) sur l'Arduino, commande basse interne des Dynamixel XL330.
// Déroulé : image RLE sur Serial1 -> renvoi à la Pi sur Serial2 -> points reçus de la Pi -> bouton -> dessin -> retour à l'origine.
// Origine = position du stylo à la mise sous tension : toujours démarrer au même coin, en dehors de la zone de 60 cm.

#include <DynamixelShield.h>

using namespace ControlTableItem;

DynamixelShield dxl;

// ----- Ports Série -----
#define SERIAL_RLE_IN   Serial1  // Réception RLE initial
#define SERIAL_RLE_OUT  Serial2  // Renvoi RLE et réception des coordonnées
const long BAUD_RLE = 115200;     // programme rle du cours ; À CONFIRMER avec le câble du jury
const long BAUD_PI = 115200;
const unsigned long ATTENTE_PI_MS = 60000;   // photo + analyse sur la Pi
const long RLE_MIN_OCTETS = 7;               // plus petite image possible : 1:1:0:1

// ----- Moteurs -----
const uint8_t ID_1 = 2;          // moteur en bas à gauche (theta_1)
const uint8_t ID_2 = 1;          // moteur en bas à droite (theta_2), À VÉRIFIER
const uint8_t ID_STYLO = 3;      // moteur du stylo
const int SENS_1 = 1;            // mettre -1 si le moteur 1 tourne dans le mauvais sens
const int SENS_2 = 1;            // mettre -1 si le moteur 2 tourne dans le mauvais sens
const float TICKS_PAR_RAD = 4096.0 / (2.0 * PI);   // XL330 : 4096 pas par tour

// ----- Stylo (positions en pas, à régler sur ton robot) -----
const float STYLO_HAUT = 2048;
const float STYLO_BAS = 2348;
const unsigned long ATTENTE_STYLO_MS = 300;   // temps pour lever ou baisser le stylo

// ----- Robot -----
const float R = 0.637;           // rayon primitif de la poulie GT2 20 dents (cm)

// ----- Dessin -----
const float V_DESSIN = 2.0;      // vitesse stylo baissé (cm/s)
const float V_VIDE = 3.5;        // vitesse stylo levé (cm/s) : environ 75 tr/min en diagonale, M288 limité vers 100 à vide
const float TOLERANCE = 0.05;    // écart accepté en fin de déplacement (cm)

// ----- Commande haute -----
const float KP = 15.0;           // gain de correction en position (1/s)
const unsigned long TE_MS = 30;  // période d'échantillonnage (ms)

// ----- Structure pour les mouvements reçus -----
const int MAX_MOVEMENTS = 800;
struct Movement {
  uint8_t etatStylo; // 1 pour baissé, 0 pour levé
  int16_t x;         // centièmes de cm
  int16_t y;
};
Movement movements[MAX_MOVEMENTS];
int nbMovements = 0;

// ----- Variables de la commande -----
float pos0_1, pos0_2;            // positions brutes au démarrage (= origine)
float theta_d1 = 0;              // consignes d'angle (sortie de l'intégrateur, rad)
float theta_d2 = 0;
float x_cible = 0;               // point atteint à la fin du dernier déplacement (cm)
float y_cible = 0;

// ----- DEBUG -----
const int ID_TONE = 48;

// ----- Bouton de démarrage (entre PIN_BOUTON et GND) -----
const bool UTILISER_BOUTON = false;   // passer à true une fois le bouton câblé
const int PIN_BOUTON = 22;

// ===================== Transfert RLE et Réception Coordonnées =====================

void echangerDonnees() {
  while (SERIAL_RLE_OUT.available()) SERIAL_RLE_OUT.read();   // réponse tardive d'une image précédente

  // 1) Renvoi de la chaîne RLE au fil de l'eau, sans la garder en mémoire (jusqu'à 8 Ko pour du 64 x 64)
  long nbOctets = 0;
  unsigned long dernierOctet = millis();
  while (millis() - dernierOctet < 1000) {
    if (!SERIAL_RLE_IN.available()) continue;
    char c = SERIAL_RLE_IN.read();
    dernierOctet = millis();
    if (c == '\n') break;
    if (c == '\r') continue;
    SERIAL_RLE_OUT.write(c);
    nbOctets++;
  }
  SERIAL_RLE_OUT.write('\n');

  nbMovements = 0;
  if (nbOctets < RLE_MIN_OCTETS) return;   // parasite sur le câble, ce n'est pas une image

  // 2) Attente de la ligne de coordonnées (ligne vide = la Pi abandonne cette image)
  unsigned long debutAttente = millis();
  while (!SERIAL_RLE_OUT.available()) {
    if (millis() - debutAttente > ATTENTE_PI_MS) return;
  }

  lireCoordonnees();
}

int16_t versCentiemes(float cm) {
  return (int16_t)(cm * 100.0 + (cm < 0 ? -0.5 : 0.5));
}

// Lit etat:x,y:etat:x,y... caractère par caractère, sans garder toute la ligne en mémoire
void lireCoordonnees() {
  char champ[12];
  byte n = 0;
  byte etape = 0;  // 0 : état du stylo, 1 : x, 2 : y
  int etatStylo = 0;
  float x = 0;

  unsigned long dernierOctet = millis();
  while (millis() - dernierOctet < 1000) {
    if (!SERIAL_RLE_OUT.available()) continue;
    char c = SERIAL_RLE_OUT.read();
    dernierOctet = millis();

    if (c != ':' && c != ',' && c != '\n') {
      if (c != '\r' && n < sizeof(champ) - 1) champ[n++] = c;
      continue;
    }

    champ[n] = '\0';
    n = 0;
    if (etape == 0) {
      etatStylo = atoi(champ);
      etape = 1;
    } else if (etape == 1) {
      x = atof(champ);
      etape = 2;
    } else {
      if (nbMovements < MAX_MOVEMENTS) {
        movements[nbMovements].etatStylo = etatStylo;
        movements[nbMovements].x = versCentiemes(x);
        movements[nbMovements].y = versCentiemes(atof(champ));
        nbMovements++;
      }
      etape = 0;
    }
    if (c == '\n') return;
  }
}


// ===================== Commande haute =====================

void allerA(float xf, float yf, float v) {
  float xi = x_cible;
  float yi = y_cible;
  float T = sqrt(sq(xf - xi) + sq(yf - yi)) / v;   // durée du trajet (s)

  unsigned long t0 = millis();
  unsigned long tPrec = t0;

  while (true) {
    unsigned long maintenant = millis();
    if (maintenant - tPrec < TE_MS) continue;     
    float dt = (maintenant - tPrec) / 1000.0;      
    tPrec = maintenant;
    float t = (maintenant - t0) / 1000.0;

    float x_d, y_d, vx_d, vy_d;
    if (t < T) {
      x_d = xi + (xf - xi) * t / T;
      y_d = yi + (yf - yi) * t / T;
      vx_d = (xf - xi) / T;
      vy_d = (yf - yi) / T;
    } else {
      x_d = xf;
      y_d = yf;
      vx_d = 0;
      vy_d = 0;
    }

    float theta_1 = SENS_1 * (dxl.getPresentPosition(ID_1) - pos0_1) / TICKS_PAR_RAD;
    float theta_2 = SENS_2 * (dxl.getPresentPosition(ID_2) - pos0_2) / TICKS_PAR_RAD;

    float x = R * 0.5 * (theta_1 + theta_2);
    float y = R * 0.5 * (theta_1 - theta_2);

    if (t >= T && sqrt(sq(xf - x) + sq(yf - y)) < TOLERANCE) break;
    if (t > T + 2.0) break;

    float vx = vx_d + KP * (x_d - x);
    float vy = vy_d + KP * (y_d - y);

    float dtheta_d1 = (vx + vy) / R;
    float dtheta_d2 = (vx - vy) / R;

    theta_d1 += dtheta_d1 * dt;
    theta_d2 += dtheta_d2 * dt;

    dxl.setGoalPosition(ID_1, pos0_1 + SENS_1 * theta_d1 * TICKS_PAR_RAD);
    dxl.setGoalPosition(ID_2, pos0_2 + SENS_2 * theta_d2 * TICKS_PAR_RAD);
  }

  x_cible = xf;
  y_cible = yf;
}


// ===================== Dessin =====================

bool styloBaisse = true;   // force le premier lever au démarrage

void stylo(bool baisse) {
  if (baisse == styloBaisse) return;
  styloBaisse = baisse;
  if (baisse) dxl.setGoalPosition(ID_STYLO, STYLO_BAS);
  else        dxl.setGoalPosition(ID_STYLO, STYLO_HAUT);
  delay(ATTENTE_STYLO_MS);
}

void attendreBouton() {
  if (!UTILISER_BOUTON) return;
  tone(ID_TONE, 660, 100);
  while (digitalRead(PIN_BOUTON) == HIGH) {}
  delay(50);
}

// Sans les 3 moteurs, le robot ne démarre pas : bip grave répété
void verifierMoteurs() {
  const uint8_t ids[] = {ID_1, ID_2, ID_STYLO};
  for (int i = 0; i < 3; i++) {
    while (!dxl.ping(ids[i])) {
      tone(ID_TONE, 110, 300);
      delay(1000);
    }
  }
}

void dessinerMouvements() {
  for (int i = 0; i < nbMovements; i++) {
    bool baisser = (movements[i].etatStylo == 1);
    float x = movements[i].x / 100.0;
    float y = movements[i].y / 100.0;

    float vitesse = baisser ? V_DESSIN : V_VIDE;
    
    stylo(baisser);
    allerA(x, y, vitesse);
  }
}


// ===================== Programme principal =====================

void setup() {
  SERIAL_RLE_IN.begin(BAUD_RLE);
  SERIAL_RLE_OUT.begin(BAUD_PI);
  pinMode(PIN_BOUTON, INPUT_PULLUP);

  dxl.begin(57600);
  dxl.setPortProtocolVersion(2.0);
  verifierMoteurs();

  dxl.torqueOff(ID_1);
  dxl.torqueOff(ID_2);
  dxl.torqueOff(ID_STYLO);
  dxl.setOperatingMode(ID_1, OP_EXTENDED_POSITION);   
  dxl.setOperatingMode(ID_2, OP_EXTENDED_POSITION);
  dxl.setOperatingMode(ID_STYLO, OP_POSITION);
  dxl.setGoalPosition(ID_1, dxl.getPresentPosition(ID_1));   // pas de saut à l'activation du couple
  dxl.setGoalPosition(ID_2, dxl.getPresentPosition(ID_2));
  dxl.torqueOn(ID_1);
  dxl.torqueOn(ID_2);
  dxl.torqueOn(ID_STYLO);

  stylo(false);

  pos0_1 = dxl.getPresentPosition(ID_1);
  pos0_2 = dxl.getPresentPosition(ID_2);
  tone(ID_TONE, 440, 100);
}

void loop() {
  // Vérifie si une chaîne RLE est disponible sur Serial1
  if (SERIAL_RLE_IN.available() > 0) {
    tone(ID_TONE, 220, 50);
    echangerDonnees();      // Transfère le RLE sur Serial2 et récupère les coordonnées
    
    if (nbMovements > 0) {
      tone(ID_TONE, 880, 200);
      attendreBouton();
      dessinerMouvements();   // Exécute les mouvements reçus
      stylo(false);
      allerA(0, 0, V_VIDE);   // Retour au point de départ une fois le dessin terminé
    }
  }
}