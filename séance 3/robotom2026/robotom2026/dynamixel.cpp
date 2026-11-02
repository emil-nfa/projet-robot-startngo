#include "dynamixel.h"

const uint8_t DXL_M1 = IDMOTEUR1;
const uint8_t DXL_M2 = IDMOTEUR2;
const float DXL_PROTOCOL_VERSION = 2.0;

DynamixelShield dxl;

//This namespace is required to use Control table item names
using namespace ControlTableItem;


void dynamixel_setup(){
  // Set Port baudrate to 57600bps. This has to match with DYNAMIXEL baudrate.
  dxl.begin(57600);
  // Set Port Protocol Version. This has to match with DYNAMIXEL protocol version.
  dxl.setPortProtocolVersion(DXL_PROTOCOL_VERSION);
  // Get DYNAMIXEL information
  dxl.ping(DXL_M1);
  dxl.ping(DXL_M2);

  definirMoteurMode(DXL_M1, OP_PWM); 
  definirMoteurMode(DXL_M2, OP_PWM); 
}

void definirMoteurMode(uint8_t idMoteur, uint8_t mode) {
  // Turn off torque when configuring items in EEPROM area
  dxl.torqueOff(idMoteur);
  dxl.setOperatingMode(idMoteur, mode);
  dxl.torqueOn(idMoteur);
}

void allumerLedMoteur(uint8_t idMoteur) {
  dxl.ledOn(idMoteur);
}
void eteindreLedMoteur(uint8_t idMoteur) {
  dxl.ledOff(idMoteur);
}

void setMoteurPWM(uint8_t idMoteur, float pwm){
  LPRINTLN("PWM Moteur ",idMoteur);
  //dxl.setGoalPWM(idMoteur, pwm);
  dxl.setGoalPWM(idMoteur, pwm, UNIT_PERCENT);
  
  LPRINTLN("Present PWM(raw) : ", dxl.getPresentPWM(idMoteur)); 
  LPRINTLN("Present PWM(ratio) : ", dxl.getPresentPWM(idMoteur, UNIT_PERCENT)); 
}



void showPositions() {

}
