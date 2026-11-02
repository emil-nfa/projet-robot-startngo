#include "lineFollower.h"

#if defined(USE_RGB)
// cas de l'utilisation d'un module à 4 capteurs
MeRGBLineFollower RGBLineFollower(LINEFOLLOWER, ADDRESS2);
#elif defined(USE_DUAL)
// cas de l'utilisation d'un module à deux capteurs
MeLineFollower lineFinder(LINEFOLLOWER);
#else
// cas de l'utilisation d'un module à 6 capteurs
#endif

T_octet lfSensor; // octet contenant l'information Noir/Blanc
// dans ses bits de poids faible 
// 0000 00 S1 S2

// Capteur array :
// 00 S6 S5 S4 S3 S2 S1  

// Capteur RGB : 
// 0000 S4 S3 S2 S1  

// NB : la librairie propose un algo de fusion de données 
// pour déduire l'angle à appliquer au robot
// Elle permet d'accéder aux informations de chaque capteur par 
// RGBLineFollower.getADCValueRGB1()
// Ainsi qu'à une information binaire (blanc/noir) par RGBLineFollower.getPositionState()
// Cf. http://docs.makeblock.com/diy-platform/en/electronic-modules/sensors/rgb-line-follower.html



uint8_t Sensor_Data[3];

void lineFollower_periodic() {
	
#if defined(USE_DUAL)
  // On appelle la fonction read seulement toutes les 10 périodes
  if (compteur % 10 == 0)
    lfSensor = lineFinder.readSensors();		// dual
#elif defined(USE_RGB)    
  // PREND TROP DE TEMPS : PAS DANS LA FONCTION PERIODIQUE 
  // RGBLineFollower.loop();         // rgb
  // lfSensor = RGBLineFollower.getPositionState();
#else
    // NB : la fonction getValue perd du temps, il ne faut pas l'appeler dans le traitement périodique
    //lfSensor = getValue();						// array
#endif
}

void lineFollower_loop() {
#if defined(USE_DUAL)
  //lfSensor = lineFinder.readSensors();
#elif defined(USE_RGB)
  RGBLineFollower.loop();
  lfSensor = RGBLineFollower.getPositionState();     
#else  
  lfSensor = getValue();
#endif
}


void lineFollower_setup() {
  #ifdef USE_RGB
    RGBLineFollower.begin();
  #endif 
}

void showLfSensor() {
  // on s'intéresse aux bits de poids faible
  T_octet i;
  static T_octet lastLFS = B10000000; // valeur impossible

  if (lastLFS == lfSensor) return;
  lastLFS = lfSensor;

  // légende
  for (i = 0; i < LF_SIZE; i++) {
    PRINT("S"); PRINT(i + 1); PRINT("\t");
  }
  NL();

  // valeurs 
  for (i = 0; i < LF_SIZE; i++) {
#if defined(USE_DUAL)
    PRINT(COL(bitRead(lfSensor, LF_SIZE - 1 - i))); PRINT("\t"); 	// dual 
#elif defined(USE_RGB)
    PRINT(COL(bitRead(lfSensor, i))); PRINT("\t");   // rgb 
#else
    PRINT(COL(bitRead(lfSensor, i))); PRINT("\t");				// array
#endif
  }
  NL();

}

uint8_t getValue()
{
  long time_out_flag = 0;

  // set the pin mode to output and set the pin value to LOW
  pinMode(LFDATAPIN, OUTPUT);
  digitalWrite(LFDATAPIN, LOW);

  // wait .98 seconds then set the pin value to HIGH
  delayMicroseconds(980);
  digitalWrite(LFDATAPIN, HIGH);

  // wait 40 ms
  delayMicroseconds(40);

  // set the pin to a known good pullup value for a baseline
  pinMode(LFDATAPIN, INPUT_PULLUP);

  // wait 50 ms
  delayMicroseconds(50);

  // prepare a non-blocking timer
  time_out_flag = millis();

  // get the pin value
  while ((digitalRead(LFDATAPIN) == 0) && ((millis() - time_out_flag) < 6));
  time_out_flag = millis();

  // get another reading for the pin value
  while ((digitalRead(LFDATAPIN) == 1) && ((millis() - time_out_flag) < 6));


  for (uint8_t k = 0; k < 3; k++)
  {
    // zero out the array element
    Sensor_Data[k] = 0x00;

    for (uint8_t i = 0; i < 8; i++)
    {
      // set the timeout flag
      time_out_flag = millis();

      // wait for 6 ms (non-blocking)
      while (digitalRead(LFDATAPIN) == 0 && ((millis() - time_out_flag) < 6));

      // get the current time in microseconds
      uint32_t HIGH_level_read_time = micros();

      // reset the timeout flag
      time_out_flag = millis();

      // wait for 6ms (non-blocking)
      while (digitalRead(LFDATAPIN) == 1 && ((millis() - time_out_flag) < 6));

      // get the elapsed interval in microseconds
      HIGH_level_read_time = micros() - HIGH_level_read_time;

      // if the elapsed time is between 50-100 microseconds
      if (HIGH_level_read_time > 50 && HIGH_level_read_time < 100)
      {
        // 'or' the value into the array element using
        // a right shift by the current loop counter value
        Sensor_Data[k] |= (0x80 >> i);
      }
    }
  }

  // test to see whether the value of array element one is the
  // same as the negated value of array element zero.
  if (Sensor_Data[1] == (uint8_t)(~(uint8_t)Sensor_Data[0]))
  {
    // if the values are the same return the value of array element zero
    return Sensor_Data[0];
  }

  // Que renvoyer si le precedent test est faux ? 
}
