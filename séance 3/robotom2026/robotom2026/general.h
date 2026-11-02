#ifndef _GENERAL_H
#define _GENERAL_H

// Fichier susceptible d'être utilisé par tous les modules
// NE PAS EDITER 
#include <Arduino.h>
#include "print.h"

// Types de données 
typedef unsigned char T_octet; 

// Variables
extern volatile long compteur; 
// extern permet d'indiquer l'existence d'une variable,
// sans demander à allouer de l'espace pour la stocker dans le code courant
// => Elle devra être déclarée dans un seul des fichiers sources du projet

// Prototypes de fonctions
void general_setup(); // a appeler dans le setup() principal
void noPeriodic();  // desactive traitements périodiques (le compteur s'incrémente toujours)
void okPeriodic(); // réactive les traitements périodiques
void general_periodic(); 
void periodic() ; 

#endif _GENERAL_H
