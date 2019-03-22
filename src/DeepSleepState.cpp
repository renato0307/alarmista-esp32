#include "DeepSleepState.h"

#include <Arduino.h>
#include <ArduinoLog.h>

#include "GlobalStatus.h"
#include "Settings.h"

#define uS_TO_S_FACTOR 1000000  /* Conversion factor for micro seconds to seconds */
#define TIME_TO_SLEEP  60        /* Time ESP32 will go to sleep (in seconds) */


/* ========================================================================= 
   Private functions 
   ========================================================================= */
   
void print_wakeup_reason(){
  esp_sleep_wakeup_cause_t wakeup_reason;

  wakeup_reason = esp_sleep_get_wakeup_cause();

  switch(wakeup_reason)
  {
    case ESP_SLEEP_WAKEUP_EXT0 : Log.trace("Wakeup caused by external signal using RTC_IO\n"); break;
    case ESP_SLEEP_WAKEUP_EXT1 : Log.trace("Wakeup caused by external signal using RTC_CNTL\n"); break;
    case ESP_SLEEP_WAKEUP_TIMER : Log.trace("Wakeup caused by timer\n"); break;
    case ESP_SLEEP_WAKEUP_TOUCHPAD : Log.trace("Wakeup caused by touchpad\n"); break;
    case ESP_SLEEP_WAKEUP_ULP : Log.trace("Wakeup caused by ULP program\n"); break;
    default : Log.trace("Wakeup was not caused by deep sleep: %d\n",wakeup_reason); break;
  }
}

/* ========================================================================= 
   Public functions 
   ========================================================================= */

// deepSleepStateLoop contains all the logic of the deep sleep state
void deepSleepStateLoop()
{
  Log.notice("=> entering state: DeepSleep\n");
  
  if (globalStatus.inDeepSleep) {
      print_wakeup_reason();
  } 
  else
  {
      settingsSaveInDeepSleep(true);
  }

  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
  Log.trace("Setup ESP32 to sleep for every %s seconds\n", String(TIME_TO_SLEEP).c_str());
  
  Log.trace("Going to sleep now\n");
  Serial.flush();
  esp_deep_sleep_start();
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