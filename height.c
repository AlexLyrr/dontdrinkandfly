#include "in4073.h"

void init_height(){
  state.initPressure = pressure;
}

void heightControl(){
  // TODO: Built in P controller. To stay at level with set pressure (initPressure). Maybe taking into account saz.
  int32_t error = 0;
  int32_t liftValue = 0;
  error = pressure - state.initPressure;
  liftValue = state.controlLiftUser + state.pLift * error;
  if (liftValue > 1000){
    liftValue = 1000;
  }else if (liftValue < 0){
    liftValue = 0;
  }

  state.controlLift = liftValue;
}
