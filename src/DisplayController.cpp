#include <Adafruit_ILI9341.h>
#include "DisplayController.h"

const unsigned char wifiicon[] PROGMEM  ={ // wifi icon
  0x00, 0xff, 0x00, 0x7e, 0x00, 0x18,0x00, 0x00
};

DisplayController::DisplayController(Adafruit_ILI9341 *tft){
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
  _tft->setTextSize(1);
  _tft->print(&timeinfo, "%d/%m/%Y");
  _tft->print(" ");
  _tft->print(&timeinfo, "%H:%M");

}

void DisplayController::displayAirconStatus(String power, String temperature){
    _tft->setCursor(0, 50);
    _tft->setTextColor(0xFFFF, 0x0000);
    _tft->setTextSize(2);

    _tft->setTextColor(ILI9341_RED);
    _tft->print("Aircon status: ");
    _tft->println(power);
    
    _tft->setCursor(0, 100);
    _tft->print("Aircon temp: ");
    _tft->print(temperature);
    _tft->print((char)247); // degree symbol
    _tft->println("C");
    
    Serial.print("Aircon status: ");
    Serial.println(power);
}