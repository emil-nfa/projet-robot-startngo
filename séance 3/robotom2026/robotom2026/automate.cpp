// NE PAS EDITER 
#include "automate.h"

String strStates[] = {
  "INCONNU", "ARRET","Détection moteurs",
  "Moteur 1 Avant", "Moteur 1 Arrière",
  "Moteur 2 Avant", "Moteur 2 Arrière",
  "Suiveur de lignes"
};

T_etat currentState;

void showStates() {
  T_octet i;
  for (i = 0; i < nbStates; i++) {
    PRINT(i); PRINT(" : "); PRINTLN(strStates[i]);
  }
}

void printCurrentState() {
  // affiche l'état courant en tete de ligne, et passe un espace ensuite
  PRINT(strStates[currentState.etatCourant]);
  PRINT("\t\t");
}

void printState(T_octet numState) {
  // Afficher la chaine de caractères correspondant au numéro de l'état passé en paramètre
  PRINT(strStates[numState]);
}
