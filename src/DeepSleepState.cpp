#include "DeepSleepState.h"

#include <Arduino.h>
#include <ArduinoLog.h>

// deepSleepStateLoop contains all the logic of the deep sleep state
void deepSleepStateLoop()
{
  Log.notice("=> entering state: DeepSleep\n");
}

// deepSleepStateButtonInterrupt will return true when the sleep was 
// interrupted by a button press
bool deepSleepStateButtonInterrupt()
{
    return false;
}

// deepSleepStateTimerInterrupt will return true when the sleep was 
// interrupted by a timer interrupt
bool deepSleepStateTimerInterrupt() 
{
    return false;
}