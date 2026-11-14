#ifndef PSM_h
#define PSM_h

#include "Arduino.h"

class PSM
{
public:
  PSM(unsigned char sensePin, unsigned char controlPin, unsigned int range, int mode = RISING, unsigned char divider = 1, unsigned char interruptMinTimeDiff = 0);

  void initTimer(uint16_t delay);

  void set(unsigned int value);

  void stopAfter(long counter);

  unsigned int cps();

private:
  static inline void onInterrupt();
  static inline void onTimerInterrupt();
  void calculateSkip();
  void updateControl(bool disable);

  unsigned char _sensePin;
  unsigned char _controlPin;
  unsigned int _range;
  unsigned char _divider;
  unsigned char _dividerCounter = 0;
  unsigned char _interruptMinTimeDiff;
  volatile unsigned int _value = 0;
  volatile unsigned int _a = 0;
  volatile bool _skip = true;
  volatile long _counter = 0;
  volatile long _stopAfter = -1;
  volatile unsigned long _lastMillis = 0;

  volatile bool _psmIntervalTimerInitialized = false;
  hw_timer_t* _psmIntervalTimer = nullptr;
};

extern PSM* _thePSM;

#endif
