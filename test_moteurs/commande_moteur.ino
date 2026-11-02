// Robot dessinateur CoreXY : reçoit une image noir et blanc codée en RLE, puis la dessine
// Commande haute (jacobienne) sur l'Arduino, commande basse interne des Dynamixel XL330-M077-T
//
// Format RLE attendu (texte) : largeur hauteur n1 n2 n3 ...
//   - n1, n2, n3... = longueurs des séries de pixels, couleurs alternées, en commençant par BLANC
//     (si l'image commence par du noir, la première série vaut 0)
//   - l'image est lue ligne par ligne, de gauche à droite, de haut en bas
//   - les séparateurs (espaces, virgules, retours à la ligne) n'ont pas d'importance

#include <DynamixelShield.h>

using namespace ControlTableItem;

DynamixelShield dxl;

// ----- Réception de l'image (Serial1 : RX1 = pin 19, TX1 = pin 18) -----
#define RLE_SERIAL Serial1
const long RLE_BAUD = 115200;

// ----- Moteurs -----
const uint8_t ID_1 = 3;          // moteur 1 (theta_1)
const uint8_t ID_2 = 2;          // moteur 2 (theta_2)
const uint8_t ID_STYLO = 3;      // moteur du stylo
const int SENS_1 = 1;            // mettre -1 si le moteur 1 tourne dans le mauvais sens
const int SENS_2 = 1;            // mettre -1 si le moteur 2 tourne dans le mauvais sens
const float TICKS_PAR_RAD = 4096.0 / (2.0 * PI);   // XL330 : 4096 pas par tour

// ----- Stylo (positions en pas, à régler sur ton robot) -----
const float STYLO_HAUT = 2048;
const float STYLO_BAS = 2348;
const unsigned long ATTENTE_STYLO_MS = 300;   // temps pour lever ou baisser le stylo

// ----- Robot -----
const float R = 0.6;             // rayon de poulie (cm)

// ----- Dessin -----
const float TAILLE_PIXEL = 0.5;  // côté d'un pixel sur la feuille (cm)
const float V_DESSIN = 2.0;      // vitesse stylo baissé (cm/s)
const float V_VIDE = 5.0;        // vitesse stylo levé (cm/s)
const float TOLERANCE = 0.05;    // écart accepté en fin de déplacement (cm)

// ----- Commande haute -----
const float KP = 15.0;           // gain de correction en position (1/s)
const unsigned long TE_MS = 30;  // période d'échantillonnage (ms)

// ----- Image : liste des traits noirs (un trait = une série de pixels noirs sur une ligne) -----
const int MAX_SEGMENTS = 600;
struct Segment {
  uint16_t ligne;
  uint16_t debut;                // première colonne noire
  uint16_t fin;                  // dernière colonne noire
};
Segment segments[MAX_SEGMENTS];
int nbSegments = 0;
long largeur = 0;
long hauteur = 0;

// ----- Variables de la commande -----
float pos0_1, pos0_2;            // positions brutes au démarrage (= origine)
float theta_d1 = 0;              // consignes d'angle (sortie de l'intégrateur, rad)
float theta_d2 = 0;
float x_cible = 0;               // point atteint à la fin du dernier déplacement (cm)
float y_cible = 0;


// ===================== Réception et décodage RLE =====================

// Lit le prochain nombre reçu en ignorant les séparateurs.
// Renvoie -1 si plus rien n'arrive pendant 2 s.
long lireNombre() {
  long n = -1;
  unsigned long dernierOctet = millis();
  while (millis() - dernierOctet < 2000) {
    if (RLE_SERIAL.available()) {
      char c = RLE_SERIAL.read();
      dernierOctet = millis();
      if (c >= '0' && c <= '9') {
        if (n < 0) n = 0;
        n = n * 10 + (c - '0');
      } else if (n >= 0) {
        return n;                // fin du nombre
      }
    }
  }
  return n;                      // fin du fichier
}

// Range une série de pixels noirs dans la liste des traits
// (une série peut déborder sur la ligne suivante : on la coupe)
void ajouterNoir(long pixel, long n) {
  while (n > 0 && nbSegments < MAX_SEGMENTS) {
    long ligne = pixel / largeur;
    long colonne = pixel % largeur;
    long nSurLigne = min(n, largeur - colonne);
    segments[nbSegments].ligne = ligne;
    segments[nbSegments].debut = colonne;
    segments[nbSegments].fin = colonne + nSurLigne - 1;
    nbSegments++;
    pixel += nSurLigne;
    n -= nSurLigne;
  }
}

void recevoirImage() {
  while (!RLE_SERIAL.available()) {}   // attente du fichier

  largeur = lireNombre();
  hauteur = lireNombre();
  if (largeur <= 0 || hauteur <= 0) return;

  long total = largeur * hauteur;
  long pixel = 0;                // numéro du pixel courant
  bool noir = false;             // la première série est blanche
  while (pixel < total) {
    long n = lireNombre();
    if (n < 0) break;            // fichier incomplet
    if (noir) ajouterNoir(pixel, n);
    pixel += n;
    noir = !noir;
  }
}


// ===================== Commande haute =====================

// Déplacement en ligne droite jusqu'à (xf, yf) à la vitesse v (cm/s)
void allerA(float xf, float yf, float v) {
  float xi = x_cible;
  float yi = y_cible;
  float T = sqrt(sq(xf - xi) + sq(yf - yi)) / v;   // durée du trajet (s)

  unsigned long t0 = millis();
  unsigned long tPrec = t0;

  while (true) {
    unsigned long maintenant = millis();
    if (maintenant - tPrec < TE_MS) continue;     // on attend la période suivante
    float dt = (maintenant - tPrec) / 1000.0;      // durée réellement écoulée (s)
    tPrec = maintenant;
    float t = (maintenant - t0) / 1000.0;

    // 1) Générateur de trajectoire : ligne droite à vitesse constante
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

    // 2) Mesure des angles moteurs (rad)
    float theta_1 = SENS_1 * (dxl.getPresentPosition(ID_1) - pos0_1) / TICKS_PAR_RAD;
    float theta_2 = SENS_2 * (dxl.getPresentPosition(ID_2) - pos0_2) / TICKS_PAR_RAD;

    // 3) MCD CoreXY : position réelle du stylo (cm)
    float x = R * 0.5 * (theta_1 + theta_2);
    float y = R * 0.5 * (theta_1 - theta_2);

    // Fin du déplacement : trajet terminé et stylo arrivé (ou 2 s de sécurité)
    if (t >= T && sqrt(sq(xf - x) + sq(yf - y)) < TOLERANCE) break;
    if (t > T + 2.0) break;

    // 4) Correction en position
    float vx = vx_d + KP * (x_d - x);
    float vy = vy_d + KP * (y_d - y);

    // 5) Jacobienne inverse : vitesses angulaires de consigne (rad/s)
    float dtheta_d1 = (vx + vy) / R;
    float dtheta_d2 = (vx - vy) / R;

    // 6) Intégrateur : consignes d'angle (rad)
    theta_d1 += dtheta_d1 * dt;
    theta_d2 += dtheta_d2 * dt;

    // 7) Envoi des consignes aux Dynamixel
    dxl.setGoalPosition(ID_1, pos0_1 + SENS_1 * theta_d1 * TICKS_PAR_RAD);
    dxl.setGoalPosition(ID_2, pos0_2 + SENS_2 * theta_d2 * TICKS_PAR_RAD);
  }

  x_cible = xf;
  y_cible = yf;
}


// ===================== Dessin =====================

void stylo(bool baisse) {
  if (baisse) dxl.setGoalPosition(ID_STYLO, STYLO_BAS);
  else        dxl.setGoalPosition(ID_STYLO, STYLO_HAUT);
  delay(ATTENTE_STYLO_MS);
}

void dessinerImage() {
  for (int i = 0; i < nbSegments; i++) {
    float y  = -(float)segments[i].ligne * TAILLE_PIXEL;   // les lignes descendent
    float x0 = segments[i].debut * TAILLE_PIXEL;
    float x1 = segments[i].fin * TAILLE_PIXEL;

    allerA(x0, y, V_VIDE);       // stylo levé jusqu'au début du trait
    stylo(true);
    allerA(x1, y, V_DESSIN);     // trait
    stylo(false);
  }
}


// ===================== Programme principal =====================

void setup() {
  RLE_SERIAL.begin(RLE_BAUD);

  dxl.begin(57600);
  dxl.setPortProtocolVersion(2.0);

  dxl.torqueOff(ID_1);
  dxl.torqueOff(ID_2);
  dxl.torqueOff(ID_STYLO);
  dxl.setOperatingMode(ID_1, OP_EXTENDED_POSITION);   // plusieurs tours possibles
  dxl.setOperatingMode(ID_2, OP_EXTENDED_POSITION);
  dxl.setOperatingMode(ID_STYLO, OP_POSITION);
  dxl.torqueOn(ID_1);
  dxl.torqueOn(ID_2);
  dxl.torqueOn(ID_STYLO);

  stylo(false);

  // La position de départ devient l'origine : coin haut gauche de l'image
  pos0_1 = dxl.getPresentPosition(ID_1);
  pos0_2 = dxl.getPresentPosition(ID_2);

  recevoirImage();               // attend le fichier RLE (t = 0) et le décode
  dessinerImage();               // démarre le dessin automatiquement
  allerA(0, 0, V_VIDE);          // retour au point de départ
}

void loop() {
  // tout se passe dans setup() : le dessin n'est fait qu'une fois
}