#include <Adafruit_ILI9341.h>
#include "DisplayController.h"
#include "aircon.h"
#include <FlickerFreePrint.h>
#include <Fonts/FreeSans9pt7b.h>

// setup some colors
#define  C_BLACK      0x0000
#define C_BLUE        0x001F
#define C_RED         0xF800
#define C_GREEN       0x07E0
#define C_CYAN        0x07FF
#define C_MAGENTA     0xF81F
#define C_YELLOW      0xFFE0
#define C_WHITE       0xFFFF

const unsigned char wifiicon[] PROGMEM  ={ // wifi icon
  0x00, 0xff, 0x00, 0x7e, 0x00, 0x18,0x00, 0x00
};

//const int MODE_FAN = 0;
//const int MODE_HEAT = 1;
//const int MODE_COLD = 2;
//const int MODE_AUTO = 3;
//const int MODE_DRY = 7;

FlickerFreePrint<Adafruit_ILI9341> _Data3(C_WHITE, C_BLACK);

DisplayController::DisplayController(Adafruit_ILI9341 *tft){
    _Data3.setDisplay(tft);
    _tft = tft;
}

void DisplayController::startDisplay(){
  _tft->begin();
}

void DisplayController::displayWifiIcon(int color){
  _tft->drawBitmap(1,1,wifiicon,8,8,color);
}

void DisplayController::displayLocalTime(tm timeinfo) {
  int startPoint = 20;
  _tft->setCursor(startPoint, 0);
  byte h, m, s;
  char str[30];
  long curtime, thetime;
  thetime = millis();
  h = (thetime / 3600000) / 1000;
  m = (thetime / 1000 - (3600 * h)) / 60;
  s = (thetime / 1000 - (3600 * h) - (m * 60));
  sprintf(str, "Time: %02d:%02d:%02d", h, m, s);

  Serial.println(str);

  _Data3.print(str);

}

void DisplayController::displayAirconStatus(controlInfo ci){
    _tft->setFont(&FreeSans9pt7b);
    _tft->setCursor(0, 30);
    _tft->setTextColor(ILI9341_GREEN, ILI9341_BLACK);
    _tft->print("Aircon status: ");
    _tft->println(ci.power);
    
    _tft->setCursor(0, 60);
    _tft->print("Aircon temp: ");
    _tft->print(ci.temperature);
    _tft->print((char)247); // degree symbol
    _tft->println("C");


    _tft->setCursor(0, 90);
    _tft->print("Aircon Mode: ");
    
    int mode = ci.mode.toInt();
    switch (mode)
    {
    case 0:
      _tft->println("Fan");
      break;
    case 1:
      _tft->println("Heat");
      break;
    case 2:
      _tft->println("Cold");
      break;
    case 3:
      _tft->println("Auto");
      break;
    case 7:
      _tft->println("Dry");
      break;
    default:
      _tft->println("Undefined");
      break;
    }
    
    
}

void DisplayController::displayZoneStatus(zonesStatusStruct zns){
    _tft->setFont(&FreeSans9pt7b);
    _tft->setTextColor(ILI9341_WHITE, ILI9341_BLACK);
    int printZoneCount=0;
    for (size_t i = 0; i < numberOfZones; i++)
    {
      if(zns.zoneNames[i]=="empty"){
        continue;
      }
      _tft->setCursor(0, 110 + printZoneCount*20);
      _tft->print(zns.zoneNames[i]+" : ");
      _tft->println(zns.zoneStatus[i]);
      printZoneCount++;
    }
    
}