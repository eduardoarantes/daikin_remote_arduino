// Learn about the ESP32 WiFi simulation in
// https://docs.wokwi.com/guides/esp32-wifi

// MOSI: 23
// MISO: 19
// SCK: 18
// SS: 5

#if defined(ESP32)
#include <WiFi.h>
#include <HTTPClient.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#else
  #error Unsupported board selection.
#endif


#include <OneButton.h>

#include <Wire.h>
#include <AsyncDelay.h>

#include "Aircon.h"
#include "DisplayController.h"

#include "SPI.h"
#include "Adafruit_GFX.h"
#include "Adafruit_ILI9341.h"

#if defined(WOKWI_SIMULATION)
#define TFT_DC 25
#define TFT_CS 5
#else
  #error PLEASE redifine pins
#endif
/*
   #define TFT_CS    D2          // TFT CS  pin is connected to NodeMCU pin D2
   #define TFT_RST   D3          // TFT RST pin is connected to NodeMCU pin D3
   #define TFT_DC    D4   

*/

Adafruit_ILI9341 display = Adafruit_ILI9341(TFT_CS, TFT_DC, 3);

DisplayController displayController = DisplayController(&display);

AsyncDelay delay_5s;
AsyncDelay delay_60s;

#define NTP_SERVER     "pool.ntp.org"
#define UTC_OFFSET     10 * 60 * 60
#define UTC_OFFSET_DST 0

#define AIRCON_IP "192.168.1.4"

#define BUTTON_POWER_STATUS 21

OneButton powerStatusButton = OneButton(
  BUTTON_POWER_STATUS,  // Input pin for the button
  true,        // Button is active LOW
  true         // Enable internal pull-up resistor
);

int myZoneId = 7;

const int rgbPins[] = { 14, 12, 13};  // PWM pins for Red, Green and Blue colors of RGB led.

zonesStatusStruct zS;
controlInfo ci;

void refresh_states_from_aircon(){
    zS = readZoneStatus();
    ci = readControlInfo();
}

void powerButtonChanged(const int buttonState){
  Serial.print("powerButtonChanged: ");
  Serial.println(buttonState);

  // verify the aircon status
  // if it's ON we need to switch the status of this zone
  // if it's OFF we need to switch all the other zones OFF and this one ON
  if(buttonState == 1){
    if(ci.power == POWER_ON){
      Serial.println("Aircon is ON"); 
      if(zS.zoneStatus[myZoneId] == POWER_ON){
        Serial.print("Setting OFF to zone:"); 
        Serial.println(myZoneId); 
        zS.zoneStatus[myZoneId]=POWER_OFF;
        // if no other zone is active, shut it down
        if(!hasActiveZone(zS)){
          Serial.println("Turning Power OFF"); 
          setPowerStatus(ci, POWER_OFF);
        }else{
          Serial.println("Keeping Power ON"); 
        }
      }else{
        Serial.print("Setting ON to zone:"); 
        Serial.println(myZoneId); 
        zS.zoneStatus[myZoneId]=POWER_ON;
      }
    }else{
      Serial.println("Aircon is off, resetting all zones");
      for(int i = 0; i < numberOfZones; i++)       {
          zS.zoneStatus[i]=POWER_OFF;
      }
      zS.zoneStatus[myZoneId]=POWER_ON;
      setPowerStatus(ci, POWER_ON);
    }
    Serial.print("Setting My zone status to:");
    Serial.println(zS.zoneStatus[myZoneId]);
    setZoneStatus(zS);
  }
}


void displayWifiStatus(){
  if(WiFi.status() == WL_CONNECTED) {
    displayController.displayWifiIcon(ILI9341_GREEN);
  }else{
    displayController.displayWifiIcon(ILI9341_WHITE);
  }
}
void setup() {

  Serial.begin(115200);
  Serial.println("setup 1");

  displayController.startDisplay();

  WiFi.begin("Wokwi-GUEST", "", 6);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.println("trying to connect to GUEST");
    displayController.displayWifiIcon(ILI9341_ORANGE);
    delay(250);
    displayController.displayWifiIcon(ILI9341_WHITE);
  }

  displayController.displayWifiIcon(ILI9341_GREEN);

  delay_5s.start(5000, AsyncDelay::MILLIS);
  delay_60s.start(60000, AsyncDelay::MILLIS);


  // powerStatusButton.setCallback(powerButtonChanged);
  powerStatusButton.attachClick([]() {
    Serial.println("Cliked Pressed!");
  }); 

  powerStatusButton.attachDoubleClick([]() {
    Serial.println("Double Pressed!");
  });
  powerStatusButton.setPressTicks(3000);
  powerStatusButton.attachLongPressStop([]() {
    Serial.println("Long Pressed!");
  });

  controlInfo ci = readControlInfo();

  displayController.displayAirconStatus(ci);
  displayWifiStatus();

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  configTime(UTC_OFFSET, UTC_OFFSET_DST, NTP_SERVER);
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Couldn't get the time from server");
  }
  displayController.displayLocalTime(timeinfo);

  zonesStatusStruct zns= readZoneStatus();

  zns.zoneStatus[0] = POWER_OFF;

  setZoneStatus(zns);

  ci.temperature = "22";

  ci.fanRate = "1";

  ci.mode = MODE_COLD;

  sendControlInfo(ci);
  
}
void loop() {

  powerStatusButton.tick();
  
  if (delay_5s.isExpired()) {
    Serial.println("Fetching aircon status");
    displayWifiStatus();
    controlInfo ci = readControlInfo();
    displayController.displayAirconStatus(ci);

    zonesStatusStruct zns= readZoneStatus();
    displayController.displayZoneStatus(zns);

    delay_5s.repeat(); // Count from when the delay expired, not now
        struct tm timeinfo;
    if (!getLocalTime(&timeinfo)) {
      Serial.println("Connection Err");
    }else{
      displayController.displayLocalTime(timeinfo);
    }

  }

  if(delay_60s.isExpired()){
    delay_60s.repeat(); // Count from when the delay expired, not now
  }

}
