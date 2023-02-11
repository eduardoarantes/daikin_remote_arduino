// Learn about the ESP32 WiFi simulation in
// https://docs.wokwi.com/guides/esp32-wifi

// MOSI: 23
// MISO: 19
// SCK: 18
// SS: 5

#include <WiFi.h>
#include <HTTPClient.h>

#include <Wire.h>
#include <AsyncDelay.h>

//https://github.com/maykon/ButtonDebounce
#include <ButtonDebounce.h>

#include "Aircon.h"

#include "SPI.h"
#include "Adafruit_GFX.h"
#include "Adafruit_ILI9341.h"

#define TFT_DC 25
#define TFT_CS 5
Adafruit_ILI9341 display = Adafruit_ILI9341(TFT_CS, TFT_DC);


AsyncDelay delay_5s;
AsyncDelay delay_60s;

#define NTP_SERVER     "pool.ntp.org"
#define UTC_OFFSET     10 * 60 * 60
#define UTC_OFFSET_DST 0

#define AIRCON_IP "192.168.1.4"

#define BUTTON_POWER_STATUS 21

ButtonDebounce powerStatusButton(BUTTON_POWER_STATUS, 300);

int myZoneId = 7;

const int rgbPins[] = { 14, 12, 13};  // PWM pins for Red, Green and Blue colors of RGB led.

void powerButtonChanged(const int buttonState){
  Serial.print("powerButtonChanged: ");
  Serial.println(buttonState);

  // verify the aircon status
  // if it's ON we need to switch the status of this zone
  // if it's OFF we need to switch all the other zones OFF and this one ON
  if(buttonState == 1){
  
    zonesStatusStruct zS = readZoneStatus();
    
    controlInfo ci = retrieveControlInfo();

    if(ci.power == ON){
      Serial.println("Aircon is ON"); 
      if(zS.zoneStatus[myZoneId] == ON){
        Serial.print("Setting OFF to zone:"); 
        Serial.println(myZoneId); 
        zS.zoneStatus[myZoneId]=OFF;
        // if no other zone is active, shut it down
        if(!hasActiveZone(zS)){
          Serial.println("Turning Power OFF"); 
          setPowerStatus(ci, OFF);
        }else{
          Serial.println("Keeping Power ON"); 
        }
      }else{
        Serial.print("Setting ON to zone:"); 
        Serial.println(myZoneId); 
        zS.zoneStatus[myZoneId]=ON;
      }
    }else{
      Serial.println("Aircon is off, resetting all zones");
      for(int i = 0; i < numberOfZones; i++)       {
          zS.zoneStatus[i]=OFF;
      }
      zS.zoneStatus[myZoneId]=ON;
      setPowerStatus(ci, ON);
    }
    Serial.print("Setting My zone status to:");
    Serial.println(zS.zoneStatus[myZoneId]);
    setZoneStatus(zS);
  }
}

const unsigned char wifiicon[] PROGMEM  ={ // wifi icon
  0x00, 0xff, 0x00, 0x7e, 0x00, 0x18,0x00, 0x00
};

void drawWifiIcon(Adafruit_ILI9341 d, int color){
  d.drawBitmap(1,1,wifiicon,8,8,color);
}


void printLocalTime(Adafruit_ILI9341 d) {

  int startPoint = 20;
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Connection Err");
    return;
  }

  d.setCursor(startPoint, 0);
  d.setTextSize(1);
  d.print(&timeinfo, "%d/%m/%Y");
  d.print(" ");
  d.print(&timeinfo, "%H:%M");

}

void displayWifiStatus(Adafruit_ILI9341 d){
  if(WiFi.status() == WL_CONNECTED) {
    drawWifiIcon(display, ILI9341_GREEN);
  }else{
    drawWifiIcon(display, ILI9341_WHITE);
  }
}

void displayAirconStatus(Adafruit_ILI9341 d, controlInfo ci){
    d.setCursor(0, 50);
    d.setTextColor(0xFFFF, 0x0000);
    d.setTextSize(2);

    d.setTextColor(ILI9341_RED);
    d.print("Aircon status: ");
    d.println(ci.power);
    
    d.setCursor(0, 100);
    d.print("Aircon temp: ");
    d.print(ci.temperature);
    d.print((char)247); // degree symbol
    d.println("C");
    
    Serial.print("Aircon status: ");
    Serial.println(ci.power);
}

void setup() {

  Serial.begin(115200);
  Serial.println("setup ");

  WiFi.begin("Wokwi-GUEST", "", 6);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.println("trying to connect to GUEST");
    drawWifiIcon(display, ILI9341_ORANGE);
    delay(250);
    drawWifiIcon(display, ILI9341_WHITE);
  }

  drawWifiIcon(display, ILI9341_GREEN);
  
  delay_5s.start(5000, AsyncDelay::MILLIS);
  delay_60s.start(60000, AsyncDelay::MILLIS);

  display.begin();

  powerStatusButton.setCallback(powerButtonChanged);

  displayAirconStatus(display, retrieveControlInfo());
  displayWifiStatus(display);

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  configTime(UTC_OFFSET, UTC_OFFSET_DST, NTP_SERVER);
  printLocalTime(display);
  
}

void loop() {
  powerStatusButton.update();
  
  if (delay_5s.isExpired()) {
    displayWifiStatus(display);
    displayAirconStatus(display, retrieveControlInfo());
    delay_5s.repeat(); // Count from when the delay expired, not now
    Serial.println("5s");
  }

  if(delay_60s.isExpired()){
    printLocalTime(display);
    delay_60s.repeat(); // Count from when the delay expired, not now
  }

}