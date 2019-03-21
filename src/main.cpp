#include <Arduino.h>
#include <ArduinoLog.h>
#include <StateMachine.h>

#include "Settings.h"
#include "ConfigurationState.h"
#include "UnimplementedStates.h"

const int STATE_DELAY = 1000;

StateMachine machine = StateMachine();
State *configurationState = machine.addState(&configurationStateLoop);
State *deepSleepState = machine.addState(&loopDeepSleepState);
State *sunriseState = machine.addState(&loopSunriseState);

void setup()
{
  Serial.begin(115200);

  Log.begin(LOG_LEVEL_TRACE, &Serial, true);

  settingsInit();

  configurationState->addTransition(&configurationStateActivateSleep, deepSleepState);
  deepSleepState->addTransition(&deepSleepButtonInterrupt, configurationState);
  deepSleepState->addTransition(&deepSleepTimerInterrupt, sunriseState);
  sunriseState->addTransition(&sunriseButtonPress, configurationState);
  sunriseState->addTransition(&sunriseAlarmTimeout, configurationState);
}

void loop()
{
  machine.run();
  delay(STATE_DELAY);
}