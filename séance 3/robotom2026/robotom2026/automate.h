#ifndef _AUTOMATE_H
#define _AUTOMATE_H

#include <Arduino.h>
#include "general.h"

// Définition des états du programme

// permet d'éviter de créer un grand nombre de constantes symboliques
//              0           1     2     3      4     5    6       7              8
enum states {INCONNU = 0, ARRET, DETECT, M1AV, M1AR, M2AV, M2AR, LINE_FOLLOWER, nbStates};
extern String strStates[]; 

// NE PAS EDITER SOUS CETTE LIGNE /////////////////////

// Pitches
#define NOTE_C4  262
#define NOTE_CS4 277
#define NOTE_D4  294
#define NOTE_DS4 311
#define NOTE_E4  330
#define NOTE_F4  349
#define NOTE_FS4 370
#define NOTE_G4  392
#define NOTE_GS4 415
#define NOTE_A4  440
#define NOTE_AS4 466
#define NOTE_B4  494

#define TRANSITION_TEMPORELLE 0
#define TRANSITION_EVENEMENTIELLE 1

#define NON_INITIALISE 0
#define A_POURSUIVRE_LOOP 1 // si la fn periodique  initialise le chgt d'état et qu'il faut le terminer dans loop 
#define A_POURSUIVRE_PERIODIC 2 // si la fn loop initialise le chgt d'état et qu'il faut le terminer dans periodic
#define INITIALISE 3

// Structure pour l'automate à états
typedef struct {
  T_octet etatPrecedent;
  T_octet etatCourant;
  T_octet etatSuivant;
  T_octet initialisation; // gestion de l'entree dans l'état
  // T_octet desactivation; // une gestion de la sortie de l'état serait-elle intéressante à mettre en place ?
  T_octet typeTransition; // temporelle, evenementielle
  long dateEntree; // valeur du compteur à l'entrée dans l'état
  long duree; // duree max dans l'état si transition temporelle, en ms
} T_etat;

void showStates(); 
void printCurrentState(); 
void printState(T_octet numState) ; 

#endif 