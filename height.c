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


void heightControl2(){
  int32_t errorPr = 0;
  int32_t currentPressure = 0;
  currentPressure = maPressureFilter();
  int32_t errorsaz = 0;
  int32_t liftValue = 0;
  errorPr = (currentPressure - state.initPressure);
  errorsaz = 0 - ((saz - state.calibrateSazOffset) >> 7);
  liftValue = state.controlLiftUser + state.pLift * errorPr + state.psaz * errorsaz;
  if (liftValue > 1000){
    liftValue = 1000;
  }else if (liftValue < 0){
    liftValue = 0;
  }

  state.controlLift = liftValue;
}
