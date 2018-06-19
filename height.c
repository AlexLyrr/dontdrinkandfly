#include "in4073.h"

void init_height(){
  state.initPressure = pressure;
  for(int i = 0; i < 30; i += 1) {
    maPressureFilter();
  }
}

/**
 * @author Roy Blokker
 */
void heightControl(){
  int32_t errorPr = 0;
  int32_t currentPressure = 0;
  currentPressure = maPressureFilter();
  int32_t errorsaz = 0;
  int32_t liftValue = 0;
  errorPr = currentPressure - state.initPressure;
  errorsaz = 0 - (saz - state.calibrateSazOffset);
  liftValue = ((state.pLift * errorPr) >> 3) + ((state.psaz * errorsaz) >> 8);
  if (liftValue > 100){
    liftValue = 100;
  }else if (liftValue < -100){
    liftValue = -100;
  }
  if (state.controlLiftUser < 200) {
    state.controlLift = 0;
    return;
  }
  state.controlLift = state.controlLiftUser + liftValue;
}
