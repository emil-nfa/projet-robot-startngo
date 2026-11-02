#include <Arduino.h>
#include <MeMegaPi.h>
#include "MeRGBLineFollower.h"

#define LF_SIZE 4

// Allume <=> 1 <=> BLANC
#define BLANC 1
#define NOIR 0

#define COL(s) ((s == BLANC) ? "B" : "N")

MeRGBLineFollower RGBLineFollower(PORT_7, ADDRESS2);

unsigned char lfSensor; // octet contenant l'information Noir/Blanc
// dans ses bits de poids faible 
// 0000 S4 S3 S2 S1  


void setup() {
  RGBLineFollower.begin();
  Serial.begin(9600);
}

void loop() {

  RGBLineFollower.loop();
  
  // int turnoffset = RGBLineFollower.getPositionOffset();
  // Serial.print("Offset : "); 
  // Serial.println(turnoffset); 
  
  lfSensor = RGBLineFollower.getPositionState();
  showLfSensor(); 
}

void showLfSensor() {
  // on s'intéresse aux bits de poids faible
  unsigned char i;
  static unsigned char lastLFS = B10000000; // valeur impossible

  if (lastLFS == lfSensor) return;
  lastLFS = lfSensor;

  // légende
  for (i = 0; i < LF_SIZE; i++) {
    Serial.print("S"); Serial.print(i + 1); Serial.print("\t");
  }
  Serial.println();

  // valeurs 
  for (i = 0; i < LF_SIZE; i++) {
    Serial.print(COL(bitRead(lfSensor, i))); Serial.print("\t");   // rgb 
  }
  Serial.println();

}
