#include <Adafruit_ILI9341.h>
#include "DisplayController.h"
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

FlickerFreePrint<Adafruit_ILI9341> _Data3(C_WHITE, C_BLACK);

DisplayController::DisplayController(Adafruit_ILI9341 *tft){
    _Data3.setDisplay(tft);
    _tft = tft;
}

void DisplayController::displayTemperature(int temperature, uint16_t color) {
  _tft->setCursor(0, 0);
  _tft->println("Temperature: " + String(temperature) + " C");
}

void DisplayController::drawWifiIcon(int color){
  _tft->drawBitmap(1,1,wifiicon,8,8,color);
}

void DisplayController::printLocalTime(tm timeinfo) {
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
  _Data3.print(str);

}

void DisplayController::displayAirconStatus(String power, String temperature){
    _tft->setFont(&FreeSans9pt7b);
    _tft->setCursor(0, 50);
    _tft->setTextColor(ILI9341_WHITE, ILI9341_BLACK);

    _tft->setTextColor(ILI9341_GREEN, ILI9341_BLACK);
    _tft->print("Aircon status: ");
    _tft->println(power);
    
    _tft->setCursor(0, 100);
    _tft->print("Aircon temp: ");
    _tft->print(temperature);
    _tft->print((char)247); // degree symbol
    _tft->println("C");
    
    // Serial.print("Aircon status: ");
    //Serial.println(power);
}