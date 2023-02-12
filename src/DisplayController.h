#ifndef DisplayController_h
#define DisplayController_h

#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>

class DisplayController {
  public:
    DisplayController(Adafruit_ILI9341 *tft);
    void displayTemperature(int temperature, uint16_t color);
    void drawWifiIcon(int color);
    void printLocalTime();
    void displayAirconStatus(String power, String temperature);
    void printLocalTime(tm timeinfo);
  private:
    Adafruit_ILI9341 *_tft;
};

#endif
