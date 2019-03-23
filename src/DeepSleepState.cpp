#include "DeepSleepState.h"

#include <Arduino.h>
#include <ArduinoLog.h>

#include "GlobalStatus.h"
#include "Settings.h"

#define uS_TO_S_FACTOR 1000000 /* Conversion factor for micro seconds to seconds */
#define TIME_TO_SLEEP 300      /* Time ESP32 will go to sleep (in seconds) */

/* ========================================================================= 
   Private functions 
   ========================================================================= */

void printWakeupReason()
{
  esp_sleep_wakeup_cause_t wakeup_reason;

  wakeup_reason = esp_sleep_get_wakeup_cause();

  switch (wakeup_reason)
  {
  case ESP_SLEEP_WAKEUP_EXT0:
    Log.trace("wakeup caused by external signal using RTC_IO\n");
    break;
  case ESP_SLEEP_WAKEUP_EXT1:
    Log.trace("wakeup caused by external signal using RTC_CNTL\n");
    break;
  case ESP_SLEEP_WAKEUP_TIMER:
    Log.trace("wakeup caused by timer\n");
    break;
  case ESP_SLEEP_WAKEUP_TOUCHPAD:
    Log.trace("wakeup caused by touchpad\n");
    break;
  case ESP_SLEEP_WAKEUP_ULP:
    Log.trace("wakeup caused by ULP program\n");
    break;
  default:
    Log.trace("wakeup was not caused by deep sleep: %d\n", wakeup_reason);
    break;
  }
}

/* ========================================================================= 
   Public functions 
   ========================================================================= */

// deepSleepStateLoop contains all the logic of the deep sleep state
void deepSleepStateLoop()
{
  Log.notice("=> entering state: DeepSleep\n");

  if (globalStatus.inDeepSleep)
  {
    Log.trace("waking up...\n");
    printWakeupReason();

    esp_sleep_wakeup_cause_t wakeup_reason;
    wakeup_reason = esp_sleep_get_wakeup_cause();
    if (wakeup_reason == ESP_SLEEP_WAKEUP_EXT0)
    {
      xSemaphoreTake(globalStatus.sleepMutex, portMAX_DELAY);
      globalStatus.goToConfig = true;
      settingsSaveInDeepSleep(false);
      xSemaphoreGive(globalStatus.sleepMutex);
      return;
    }
    if (wakeup_reason == ESP_SLEEP_WAKEUP_TIMER)
    {
      xSemaphoreTake(globalStatus.sleepMutex, portMAX_DELAY);
      globalStatus.goToSunrise = true;
      settingsSaveInDeepSleep(false);
      xSemaphoreGive(globalStatus.sleepMutex);
      return;
    }
  }

  Log.trace("going to sleep now\n");
  settingsSaveInDeepSleep(true);
  esp_sleep_enable_ext0_wakeup(GPIO_NUM_13, 1);
  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
  Log.trace("setup ESP32 to sleep for every %s seconds\n", String(TIME_TO_SLEEP).c_str());

  esp_deep_sleep_start();
}

// deepSleepStateButtonInterrupt will return true when the sleep was
// interrupted by a button press
bool deepSleepStateButtonInterrupt()
{
  xSemaphoreTake(globalStatus.sleepMutex, portMAX_DELAY);

  Log.trace("going to config? %b\n", globalStatus.goToConfig);
  bool goToConfigFlag = globalStatus.goToConfig;
  globalStatus.goToConfig = false;
  xSemaphoreGive(globalStatus.sleepMutex);

  return goToConfigFlag;
}

// deepSleepStateTimerInterrupt will return true when the sleep was
// interrupted by a timer interrupt
bool deepSleepStateTimerInterrupt()
{
  xSemaphoreTake(globalStatus.sleepMutex, portMAX_DELAY);

  Log.trace("going to sunrise? %b\n", globalStatus.goToSunrise);
  bool goToSunriseFlag = globalStatus.goToSunrise;
  globalStatus.goToSunrise = false;
  xSemaphoreGive(globalStatus.sleepMutex);

  return goToSunriseFlag;
}