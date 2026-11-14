#include "PSM.h"

PSM* _thePSM;

PSM::PSM(unsigned char sensePin, unsigned char controlPin, unsigned int range, int mode, unsigned char divider, unsigned char interruptMinTimeDiff) {
  _thePSM = this;

  pinMode(sensePin, INPUT_PULLUP);
  PSM::_sensePin = sensePin;

  pinMode(controlPin, OUTPUT);
  PSM::_controlPin = controlPin;

  PSM::_divider = divider > 0 ? divider : 1;

  uint8_t interruptNum = digitalPinToInterrupt(PSM::_sensePin);

  updateControl(/*disable=*/true);

  if (interruptNum != NOT_AN_INTERRUPT) {
    attachInterrupt(interruptNum, onInterrupt, mode);
  }

  PSM::_range = range;
  PSM::_interruptMinTimeDiff = interruptMinTimeDiff;

  //initTimer(1500);
}

void onPSMInterrupt() __attribute__((weak));
void onPSMInterrupt() {}

void PSM::onInterrupt() {
  if (_thePSM->_interruptMinTimeDiff > 0 && millis() - _thePSM->_lastMillis < _thePSM->_interruptMinTimeDiff) {
    return;
  }
  _thePSM->_lastMillis = millis();

  onPSMInterrupt();

  _thePSM->_dividerCounter++;
  _thePSM->_dividerCounter %= _thePSM->_divider;

  if (_thePSM->_dividerCounter == 0) {
    _thePSM->calculateSkip();
  }
}

void PSM::onTimerInterrupt() {
  timerStop(_thePSM->_psmIntervalTimer);
  _thePSM->updateControl(/*disable=*/true);
}

void PSM::set(unsigned int value) {
  PSM::_value = value < PSM::_range ? value : PSM::_range;
}

void PSM::stopAfter(long counter) {
  PSM::_stopAfter = counter;
}

void PSM::calculateSkip() {
  PSM::_a += PSM::_value;

  bool skip = true;
  
  if (PSM::_a >= PSM::_range) {
    PSM::_a -= PSM::_range;
    skip = false; 
    
  }

  updateControl(skip);
}

void PSM::updateControl(bool disable) {
  if (disable) {
    digitalWrite(PSM::_controlPin, LOW);
  } else {
    digitalWrite(PSM::_controlPin, HIGH);

    if (_thePSM->_psmIntervalTimerInitialized) {
      timerRestart(_thePSM->_psmIntervalTimer);      // counter = 0
      timerAlarmEnable(_thePSM->_psmIntervalTimer);  // <-- re-arm; hardware cleared it on last fire
      timerStart(_thePSM->_psmIntervalTimer);        // resume the paused counter
    }
    else {
      initTimer(1000);
    }
  }
}

unsigned int PSM::cps() {
  unsigned int range = PSM::_range;
  unsigned int value = PSM::_value;
  unsigned char divider = PSM::_divider;

  PSM::_range = 0xFFFF;
  PSM::_value = 1;
  PSM::_a = 0;
  PSM::_divider = 1;

  unsigned long stopAt = millis() + 1000;

  while (millis() < stopAt) {
    delay(0);
  }

  unsigned int result = PSM::_a;

  PSM::_range = range;
  PSM::_value = value;
  PSM::_a = 0;
  PSM::_divider = divider;

  //ESP_LOGI(LOG_TAG, "inner CPS: %d", result);

  return result;
}

void PSM::initTimer(uint16_t delay) {
  uint32_t us = delay;
  _psmIntervalTimer = timerBegin(0, 80, true);
  timerAttachInterrupt(_psmIntervalTimer, onTimerInterrupt, false);
  timerAlarmWrite(_psmIntervalTimer, us, true);
  timerAlarmEnable(_psmIntervalTimer);

  PSM::_psmIntervalTimerInitialized = true;
}
