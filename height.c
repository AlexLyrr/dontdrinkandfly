#include "in4073.h"

void init_height(){
  initPressure = pressure;
}

void calc_height(){
  //P1=P0 * exp(-Mg/RT1 * h1)
  //P2 = P0 * exp(-Mg/RT2 * h2)
  //P0 = exp(-Mg/RT2 * h2)/P2
  //P1=exp(-Mg/RT2 * h2) / P2 * exp(-Mg/RT1 * h1)
  //P1*P2 = exp(-Mg/RT2 *h2 + -Mg/RT1 * h1)
  //P1*P2 = exp(-Mg/R (h1/T1 + h2/T2))
  //-Mg/R (h1/T1 + h2/T2) = ln(P1*P2)
  //h1/T1 + h2/T2 = -R*ln(P1*P2)/Mg
  //h2 = T2*(-R*ln(P1*P2)/Mg - h1/T1)
}
