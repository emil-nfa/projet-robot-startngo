#ifndef _DYNAMIXEL_H
#define _DYNAMIXEL_H

#include <DynamixelShield.h>
#include "general.h"

#define IDMOTEUR1 1
#define IDMOTEUR2 2

extern const uint8_t DXL_M1;
extern const uint8_t DXL_M2;

void dynamixel_setup();
void showPositions();

void allumerLedMoteur(uint8_t idMoteur);
void eteindreLedMoteur(uint8_t idMoteur);
void setMoteurPWM(uint8_t idMoteur, float pwm);
void definirMoteurMode(uint8_t idMoteur, uint8_t mode); 


#endif