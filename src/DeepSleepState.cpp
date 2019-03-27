#include "DeepSleepState.h"

#include <Arduino.h>
#include <ArduinoLog.h>
#include <WiFi.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

#include "GlobalStatus.h"
#include "Settings.h"

#define uS_TO_S_FACTOR 1000000 /* Conversion factor for micro seconds to seconds */

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

// getSecondsToSleep returns the number of seconds to sleep based on a specific alarm.
unsigned long getSecondsToSleep(Alarm *alarm) {
  Log.trace("alarm time in seconds is %d \n", alarm->when);

  WiFiUDP ntpUDP;
  NTPClient timeClient(ntpUDP);

  if (WiFi.status() != WL_CONNECTED)
  {
    Log.error("wifi not connected, cannot fech current time\n");
    return 0;
  }

  timeClient.begin();
  // Set offset time in seconds to adjust for your timezone, for example:
  // GMT +1 = 3600
  // GMT +8 = 28800
  // GMT -1 = -3600
  // GMT 0 = 0
  timeClient.setTimeOffset(0);
  while(!timeClient.update()) {
    timeClient.forceUpdate();
  }
  unsigned long currentTime = timeClient.getEpochTime();
  Log.trace("current date time is %d (%s)\n", currentTime, timeClient.getFormattedTime());

  unsigned long totalSecondsSinceStartDay = timeClient.getHours() * 60 * 60 + timeClient.getMinutes() * 60 + timeClient.getSeconds();
  Log.verbose("total seconds since start day is %d\n", totalSecondsSinceStartDay);
  
  if (totalSecondsSinceStartDay < alarm->when) 
  {
    Log.verbose("alarm will fire today\n");
    return alarm->when - totalSecondsSinceStartDay;
  }

  Log.verbose("alarm will fire tomorrow\n");

  unsigned long startOfToday = currentTime - totalSecondsSinceStartDay;
  Log.verbose("start of today in epoch is %d\n", startOfToday);

  unsigned long tomorrowStartOfDay = startOfToday + (24*60*60);
  Log.verbose("tomorrow start of day in epoch is %d\n", tomorrowStartOfDay);

  unsigned long alarmTime = tomorrowStartOfDay + alarm->when;
  Log.verbose("alarm time in epoch is %d\n", alarmTime);

  return alarmTime - currentTime;
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
      globalStatus.goToConfig = true;
      settingsSaveInDeepSleep(false);
      return;
    }
    if (wakeup_reason == ESP_SLEEP_WAKEUP_TIMER)
    {
      globalStatus.goToSunrise = true;
      settingsSaveInDeepSleep(false);
      return;
    }
  }

  Alarm alarm = settingsGetAlarm(1);
  if (alarm.number != 1) 
  {
    Log.trace("alarm 1 not defined - going back to config.\n");
    globalStatus.goToConfig = true;
    settingsSaveInDeepSleep(false);
    return;
  }

  long timeToSleep = getSecondsToSleep(&alarm);
  if (timeToSleep == 0) 
  {
    Log.trace("invalid time to sleep returned - going back to config.\n");
    globalStatus.goToConfig = true;
    settingsSaveInDeepSleep(false);
    return;
  }

  Log.trace("going to sleep now...\n");
  settingsSaveInDeepSleep(true);
  esp_sleep_enable_ext0_wakeup(GPIO_NUM_13, 1);
  esp_err_t result = esp_sleep_enable_timer_wakeup(timeToSleep * uS_TO_S_FACTOR);

  if (result != ESP_OK) 
  {
    Log.error("error enabling timer wakeup: %d - going back to config.\n", result);
    globalStatus.goToConfig = true;
    settingsSaveInDeepSleep(false);
    return;
  }
  Log.trace("setup ESP32 to sleep for %s seconds\n", String(timeToSleep).c_str());
  esp_deep_sleep_start();
}

// deepSleepStateButtonInterrupt will return true when the sleep was
// interrupted by a button press
bool deepSleepStateButtonInterrupt()
{
  Log.trace("going to config? %b\n", globalStatus.goToConfig);
  return globalStatus.goToConfig;
}

// deepSleepStateTimerInterrupt will return true when the sleep was
// interrupted by a timer interrupt
bool deepSleepStateTimerInterrupt()
{
  Log.trace("going to sunrise? %b\n", globalStatus.goToSunrise);
  return globalStatus.goToSunrise;
}