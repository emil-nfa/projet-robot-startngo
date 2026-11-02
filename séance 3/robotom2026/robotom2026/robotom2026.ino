// NB : nécessaire pour l'utilisation des modules de suivi de ligne (cf. lineFollower)
// NB : l'utilisation de la librairie MegaPI pose pb si on utilise la fonction tone()
// Il faut supprimer les fichiers MeIR.cpp et MeIR.h du répertoire d'installation de la librairie Makeblock

#include "general.h"
#include "automate.h"
#include "lineFollower.h"
#include "dynamixel.h"
#include <MeMegaPi.h>

// Connexions du robot à l'extérieur
#define LED 13
#define BUZZER 22

int currentMotor; 
int flagShown=0;
T_octet lastStateShown = INCONNU;
extern T_etat currentState;

void setup() {

  // Exemples d'états d'initialisation possibles : 
  // etatPrecedent;  etatCourant;  etatSuivant;  initialisation; typeTransition; dateEntree;  duree;
  //T_etat initState = {INCONNU, DETECT, ARRET, NON_INITIALISE, TRANSITION_EVENEMENTIELLE, 0, 10000 }; // rester dans un etat
  T_etat initState = {INCONNU, ARRET, DETECT, NON_INITIALISE, TRANSITION_TEMPORELLE, 0, 10000}; // changer d'état toutes les 10s

  currentState = initState;
  general_setup();
  dynamixel_setup();
  lineFollower_setup();
  Serial.print("coucou");
}

void loop() {
  long inByte;
  T_etat nextState;

  // Récupération message sur port série : pourrait faire changer d'état
  if (AVAIL() > 0) {
    if (PEEK() == 10) READ(); // élimination du saut de ligne
    else {
      inByte = PARSEINT();
      PRINT("recu : "); PRINTLN(inByte);
      //ATTENTION : ce code peut être interrompu en plein milieu du changement des variables de l'état.
      // Il faudrait arrêter les interruptions et recommencer...
      // Ou empêcher les traitements de la fnPeriodique pendant ce temps...
      // On prépare le nouvel état pour gagner du temps, on rend juste l'affectation atomique
      // C'est à dire qu'au moment de changer l'état, les fonctions périodiques ne s'exécutent plus

      nextState.etatPrecedent = currentState.etatCourant;
      nextState.etatCourant = inByte;
      nextState.typeTransition = TRANSITION_EVENEMENTIELLE;   // le programme restera bloqué dans cet état jusqu'à ce qu'un événement le fasse changer
      nextState.etatSuivant = INCONNU;
      nextState.initialisation = A_POURSUIVRE_PERIODIC;
      nextState.dateEntree = compteur;
      
      noPeriodic();
      currentState = nextState; // changement d'état atomique 
      okPeriodic();

      showStates(); // affichage du menu 
    }
  }

  // Affichage de l'état lorsqu'il change, et production d'un son
  if (lastStateShown != currentState.etatCourant) {
    printState(lastStateShown);
    PRINT(" > ");
    lastStateShown = currentState.etatCourant;
    printState(lastStateShown);
    NL();
    tone(BUZZER, NOTE_A4, 30);
  }

  // Comment choisir si on doit placer son code dans loop ou periodic ?
  // dans loop si traitement long, affichages réguliers, etc.
  // dans periodic() sinon

  // Attention : parfois dans un état il ne faut rien faire :
  // On configure un comportement à l'entrée dans l'état,
  // On ne le modifie pas tant qu'on reste dans cet état

  switch (currentState.etatCourant) {
    case ARRET :
      if (currentState.initialisation == A_POURSUIVRE_LOOP) {
        //traitement à faire une fois à l'entrée dans cet état
        PRINTLN("Entrée dans l'état ARRET");
        setMoteurPWM(DXL_M1, 0);
        setMoteurPWM(DXL_M2, 0);
        currentState.initialisation = INITIALISE;
      }
      // traitement à faire à chaque itération de loop() dans cet état
      break;

    case DETECT:
      // On alterne les états des LED depuis la fonction périodique 
       if (currentState.initialisation == A_POURSUIVRE_LOOP) {
        //traitement à faire une fois à l'entrée dans cet état
        PRINTLN("Entrée dans l'état DETECT");
        currentState.initialisation = INITIALISE;
      }

      if (! flagShown) {
        // NB : l'envoi des ordres aux composants dynamixel repose sur des messages série, qui sont lents 
        // On ne peut les envoyer depuis la fonction périodique
        // => La fonction périodique change l'état, la fonction principale envoie les ordres 
        if (currentMotor == IDMOTEUR1) {
              allumerLedMoteur(DXL_M1);
              eteindreLedMoteur(DXL_M2);
          } else {
              allumerLedMoteur(DXL_M2);
              eteindreLedMoteur(DXL_M1);
          }
        LPRINTLN("Moteur allumé : ", currentMotor);
        flagShown=1; 
      }

    break;

    case M1AV :
      if (currentState.initialisation == A_POURSUIVRE_LOOP) {
        //traitement à faire une fois à l'entrée dans cet état
        PRINTLN("Entrée dans l'état M1AV");
        setMoteurPWM(DXL_M1, 80.); 
        setMoteurPWM(DXL_M2, 0);
        currentState.initialisation = INITIALISE;
      }
      // traitement à faire à chaque itération de loop() dans cet état
      showPositions();
      break;

    case M1AR :
      if (currentState.initialisation == A_POURSUIVRE_LOOP) {
        //traitement à faire une fois à l'entrée dans cet état
        PRINTLN("Entrée dans l'état M1AR");
        setMoteurPWM(DXL_M1, -80.); 
        setMoteurPWM(DXL_M2, 0);
        currentState.initialisation = INITIALISE;
      }
      // traitement à faire à chaque itération de loop() dans cet état
      showPositions();
      break;

    case M2AV :
      if (currentState.initialisation == A_POURSUIVRE_LOOP) {
        //traitement à faire une fois à l'entrée dans cet état
        PRINTLN("Entrée dans l'état M2AV");
        setMoteurPWM(DXL_M2, 80.); 
        setMoteurPWM(DXL_M1, 0);
        currentState.initialisation = INITIALISE;
      }
      // traitement à faire à chaque itération de loop() dans cet état
      showPositions();
      break;

    case M2AR :
      if (currentState.initialisation == A_POURSUIVRE_LOOP) {
        //traitement à faire une fois à l'entrée dans cet état
        PRINTLN("Entrée dans l'état M2AR");
        setMoteurPWM(DXL_M2, -80.); 
        setMoteurPWM(DXL_M1, 0);
        currentState.initialisation = INITIALISE;
      }
      // traitement à faire à chaque itération de loop() dans cet état
      showPositions();
      break;

    case LINE_FOLLOWER :
      if (currentState.initialisation == A_POURSUIVRE_LOOP) {
        //traitement à faire une fois à l'entrée dans cet état
        PRINTLN("Entrée dans l'état SUIVEUR DE LIGNES");
        setMoteurPWM(IDMOTEUR1, 0);
        setMoteurPWM(IDMOTEUR2, 0);
        currentState.initialisation = INITIALISE;
      }
      // traitement à faire à chaque itération de loop() dans cet état
	    lineFollower_loop();
      showLfSensor();      
      break;

    default :
      break;
  }

}



void periodic() {

  // Si on a atteint la duree max de maintien dans l'état, on change d'état
  if (currentState.typeTransition == TRANSITION_TEMPORELLE) {
    if (compteur > currentState.dateEntree + currentState.duree) {
      // On doit changer d'état
      currentState.etatPrecedent = currentState.etatCourant;
      currentState.etatCourant = currentState.etatSuivant;
      currentState.etatSuivant = (currentState.etatSuivant + 1) % nbStates;
      if (currentState.etatSuivant == INCONNU) currentState.etatSuivant = DETECT;
      currentState.initialisation = NON_INITIALISE;
      currentState.dateEntree = compteur;
    }
  }

  // Dans certains états, il faut réaliser des acquisitions de mesures de capteurs, réaliser des asservissements, etc.
  switch (currentState.etatCourant) {
    case ARRET :

      // entrée dans l'état :
      if (currentState.initialisation == NON_INITIALISE || currentState.initialisation == A_POURSUIVRE_PERIODIC) {
        currentState.initialisation = A_POURSUIVRE_LOOP;
      }

      // a faire dans l'état :

      break;

    case DETECT :

      // entrée dans l'état :
      if (currentState.initialisation == NON_INITIALISE || currentState.initialisation == A_POURSUIVRE_PERIODIC) {
        currentMotor = IDMOTEUR1; 
        currentState.initialisation = A_POURSUIVRE_LOOP;
      }

      // a faire dans l'état : inverser l'état des deux moteurs 
      if (compteur%2000==0) { // 1000 <=> toutes les secondes
          if (currentMotor == IDMOTEUR1) {
              currentMotor = IDMOTEUR2; 
          } else {
              currentMotor = IDMOTEUR1;
          }
          flagShown=0; 
      }

      break;

    case M1AV :
      // entrée dans l'état :
      if (currentState.initialisation == NON_INITIALISE || currentState.initialisation == A_POURSUIVRE_PERIODIC) {
        currentState.initialisation = A_POURSUIVRE_LOOP;
      }

      // a faire dans l'état :

      break;

    case M1AR :
      // entrée dans l'état :
      if (currentState.initialisation == NON_INITIALISE || currentState.initialisation == A_POURSUIVRE_PERIODIC) {
        currentState.initialisation = A_POURSUIVRE_LOOP;
      }

      // a faire dans l'état :

      break;

    case M2AV :
      // entrée dans l'état :
      if (currentState.initialisation == NON_INITIALISE || currentState.initialisation == A_POURSUIVRE_PERIODIC) {
        currentState.initialisation = A_POURSUIVRE_LOOP;
      }

      // a faire dans l'état :

      break;

    case M2AR :
      // entrée dans l'état :
      if (currentState.initialisation == NON_INITIALISE || currentState.initialisation == A_POURSUIVRE_PERIODIC) {
        currentState.initialisation = A_POURSUIVRE_LOOP;
      }

      // a faire dans l'état :

      break;

    case LINE_FOLLOWER :
      // entrée dans l'état : 
      if (currentState.initialisation == NON_INITIALISE || currentState.initialisation == A_POURSUIVRE_PERIODIC) {
        currentState.initialisation = A_POURSUIVRE_LOOP;
      }
      
      // a faire dans l'état : appeler la fonction periodic de ce capteur
      // qui s'occupera de mettre à jour, à sa propre vitesse...
      lineFollower_periodic();

      break;

    default:
      break;
  }

}
