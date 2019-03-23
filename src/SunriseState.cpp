
#include <Arduino.h>
#include <ArduinoLog.h>

#include "GlobalStatus.h"

#define BUTTON_INTERRUPT_PIN 13

/* ========================================================================= 
   Private functions 
   ========================================================================= */

// buttonInterrupt handles the sunrise interruption by a button state change
void buttonInterrupt()
{
  globalStatus.goToConfig = true;
}

/* ========================================================================= 
   Public functions 
   ========================================================================= */

// sunriseStateLoop contains all the logic of the sunrise state
void sunriseStateLoop()
{
  Log.notice("=> entering state: Sunrise\n");
  Log.trace("going to sunrise? %b\n", globalStatus.goToSunrise);

  if (globalStatus.goToSunrise) {
    Log.trace("initializing button interrupt\n");
    pinMode(BUTTON_INTERRUPT_PIN, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(BUTTON_INTERRUPT_PIN), buttonInterrupt, CHANGE);
    globalStatus.goToSunrise = false;
  }
}

// sunriseStateButtonPress returns true if any button is pressed during sunrise
bool sunriseStateButtonPress()
{
  Log.trace("going to config due to interrupt? %b\n", globalStatus.goToConfig);
  return globalStatus.goToConfig;
}

// sunriseStateAlarmTimeout returns true the alarm finishes
bool sunriseStateAlarmTimeout()
{
  return false;
}
