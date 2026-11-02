#include "general.h"

// Selection du timer à utiliser
//#define USE_TIMER_1
//#define TIMER1_PERIOD 1000

#define USE_TIMER_3
#define TIMER3_PERIOD 1000

// Pourquoi Timer3 ? Car le Timer1 est susceptible d'être utilisé pour la librairie Servo...
// Comment se procurer la librairie Timer3 ? https://github.com/PaulStoffregen/TimerThree

// NE PAS EDITER SOUS CETTE LIGNE /////////////////////

#ifdef USE_TIMER_1
#include <TimerOne.h>
#endif

#ifdef USE_TIMER_3
#include <TimerThree.h>
#endif

volatile long compteur = 0; // valeur max : 2,147,483,647
T_octet stopPeriodicCalls = 0; // séparer la gestion des deux timers ?

void general_setup() {
  
  #if (SPIN==0) 
    Serial.begin(SPINSPEED);
  #elif (SPIN==1) 
    Serial1.begin(SPINSPEED);
  #elif (SPIN==2) 
    Serial2.begin(SPINSPEED);
  #elif (SPIN==3) 
    Serial3.begin(SPINSPEED);
  #endif

  #if (SPOUT==0) 
    Serial.begin(SPOUTSPEED);
  #elif (SPOUT==1) 
    Serial1.begin(SPOUTSPEED);
  #elif (SPOUT==2) 
    Serial2.begin(SPOUTSPEED);
  #elif (SPOUT==3) 
    Serial3.begin(SPOUTSPEED);
  #endif  
  

#ifdef USE_TIMER_1
  Timer1.initialize(TIMER1_PERIOD); // appel toutes les ms
  Timer1.attachInterrupt(general_periodic);
#endif

#ifdef USE_TIMER_3
  Timer3.initialize(TIMER3_PERIOD); // appel toutes les ms
  Timer3.attachInterrupt(general_periodic);
#endif

  // Le compteur étant un "long", on dispose de plus de 2 milliards de valeurs avant de repasser 0,
  // soit 2 millions de secondes (au moins 555h de fonctionnement correct)
}

void general_periodic() {
  compteur++;
  if (stopPeriodicCalls) return ;
  periodic();  // cette fonction doit être implémentée dans le projet
}

void noPeriodic() {
  stopPeriodicCalls = 1;
}

void okPeriodic() {
  stopPeriodicCalls = 0;
}
