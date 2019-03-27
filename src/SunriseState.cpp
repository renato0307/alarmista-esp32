#include <Arduino.h>
#include <ArduinoLog.h>
#include <FastLED.h>
#include <DHTesp.h>

#include "GlobalStatus.h"

/* ========================================================================= 
   Definitions 
   ========================================================================= */

#define BUTTON_INTERRUPT_PIN 13
#define DHT_PIN 4
#define NUM_LEDS 16
#define LEDS_PIN 12

struct TemperatureAndHumidity
{
    float temperature;
    float humidity;
    float heatIndex;
    float dewPoint;
    ComfortState cf;
};

DHTesp dht;
CRGB leds[NUM_LEDS];

/* ========================================================================= 
   Private functions 
   ========================================================================= */

// buttonInterrupt handles the sunrise interruption by a button state change
void buttonInterrupt()
{
  globalStatus.goToConfig = true;
}

// getTemperatureAndHumidity reads the temperature from DHT and prints it to log
TemperatureAndHumidity getTemperatureAndHumidity() {
  // Reading temperature for humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (it's a very slow sensor)
  TempAndHumidity newValues = dht.getTempAndHumidity();
  // Check if any reads failed and exit early (to try again).
  if (dht.getStatus() != 0) {
    Log.error("DHT11 error status: %s\n", dht.getStatusString());
    return TemperatureAndHumidity { };
  }

  ComfortState cf;

  float heatIndex = dht.computeHeatIndex(newValues.temperature, newValues.humidity);
  float dewPoint = dht.computeDewPoint(newValues.temperature, newValues.humidity);
  //float cr = dht.getComfortRatio(cf, newValues.temperature, newValues.humidity);

  String comfortStatus;
  switch (cf) {
    case Comfort_OK:
      comfortStatus = "Comfort_OK";
      break;
    case Comfort_TooHot:
      comfortStatus = "Comfort_TooHot";
      break;
    case Comfort_TooCold:
      comfortStatus = "Comfort_TooCold";
      break;
    case Comfort_TooDry:
      comfortStatus = "Comfort_TooDry";
      break;
    case Comfort_TooHumid:
      comfortStatus = "Comfort_TooHumid";
      break;
    case Comfort_HotAndHumid:
      comfortStatus = "Comfort_HotAndHumid";
      break;
    case Comfort_HotAndDry:
      comfortStatus = "Comfort_HotAndDry";
      break;
    case Comfort_ColdAndHumid:
      comfortStatus = "Comfort_ColdAndHumid";
      break;
    case Comfort_ColdAndDry:
      comfortStatus = "Comfort_ColdAndDry";
      break;
    default:
      comfortStatus = "Unknown:";
      break;
  };

  Log.trace("temperature: %s\n", String(newValues.temperature).c_str());
  Log.trace("humidity: %s\n", String(newValues.humidity).c_str());
  Log.trace("heat index: %s\n", String(heatIndex).c_str());
  Log.trace("dew point: %s\n", String(dewPoint).c_str());
  Log.trace("confort status: %s\n", comfortStatus.c_str());

  return TemperatureAndHumidity { 
    newValues.temperature, 
    newValues.temperature,
    heatIndex,
    dewPoint,
    cf };
}


// sunrise simulstes the sunrise using leds.
bool sunrise() {

  static bool sunrising = true;

  // total sunrise length, in minutes
  static const uint8_t sunriseLength = 30;

  // how often (in seconds) should the heat color increase?
  // for the default of 30 minutes, this should be about every 7 seconds
  // 7 seconds x 256 gradient steps = 1,792 seconds = ~30 minutes
  static const uint8_t interval = (sunriseLength * 60) / 256;

  // current gradient palette color index
  static uint8_t heatIndex = 0; // start out at 0

  // HeatColors_p is a gradient palette built in to FastLED
  // that fades from black to red, orange, yellow, white
  // feel free to use another palette or define your own custom one
  CRGB color = ColorFromPalette(HeatColors_p, heatIndex, heatIndex);

  // fill the entire strip with the current color
  fill_solid(leds, NUM_LEDS, color);

  // slowly increase the heat
  EVERY_N_SECONDS(interval) {
    Log.trace("sunrise heat index is %d\n", heatIndex);
    heatIndex++;
    if (heatIndex == 254) {
      sunrising = false;
    }
  }

  return sunrising;
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

    Log.trace("initializing temperature sensor\n");
    dht.setup(DHT_PIN, DHTesp::DHT11);

    Log.trace("initializing leds");
    FastLED.addLeds<NEOPIXEL, LEDS_PIN>(leds, NUM_LEDS);

    getTemperatureAndHumidity();

    globalStatus.goToSunrise = false;
  }

  globalStatus.isAlarmTimeout = !sunrise();
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
  Log.trace("is alarm timeout? %b\n", globalStatus.isAlarmTimeout);
  return globalStatus.isAlarmTimeout;
}
