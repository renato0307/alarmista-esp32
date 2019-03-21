
#include <Arduino.h>

void loopDeepSleepState()
{
  Serial.println("loopDeepSleepState");
}

void loopSunriseState()
{
  Serial.println("loopSunriseState");
}


bool deepSleepButtonInterrupt()
{
  return false;
}

bool deepSleepTimerInterrupt()
{
  return false;
}

bool sunriseButtonPress()
{
  return false;
}

bool sunriseAlarmTimeout()
{
  return false;
}