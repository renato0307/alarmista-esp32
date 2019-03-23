#include <Arduino.h>
#include <ArduinoLog.h>
#include <StateMachine.h>

#include "Settings.h"
#include "ConfigurationState.h"
#include "DeepSleepState.h"
#include "SunriseState.h"

const int STATE_DELAY = 1000;

StateMachine machine = StateMachine();
State *configurationState = machine.addState(&configurationStateLoop);
State *deepSleepState = machine.addState(&deepSleepStateLoop);
State *sunriseState = machine.addState(&sunriseStateLoop);

void setup()
{
  Serial.begin(115200);

  Log.begin(LOG_LEVEL_TRACE, &Serial, true);
  Log.notice("running global setup\n");

  settingsInit();

  configurationState->addTransition(&configurationStateActivateSleep, deepSleepState);
  deepSleepState->addTransition(&deepSleepStateButtonInterrupt, configurationState);
  deepSleepState->addTransition(&deepSleepStateTimerInterrupt, sunriseState);
  sunriseState->addTransition(&sunriseStateButtonPress, configurationState);
  sunriseState->addTransition(&sunriseStateAlarmTimeout, configurationState);
}

void loop()
{
  globalStatus.inDeepSleep = settingsGetInDeepSleep();
  if (globalStatus.inDeepSleep)
  {
    Log.trace("waking up from deep sleep\n");
    machine.transitionTo(deepSleepState);
  }

  machine.run();
  delay(STATE_DELAY);
}