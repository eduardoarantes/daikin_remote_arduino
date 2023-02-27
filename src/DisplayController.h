#ifndef DisplayController_h
#define DisplayController_h

#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include "aircon.h"

class DisplayController {
  public:
    DisplayController(Adafruit_ILI9341 *tft);
    void startDisplay();
    void displayWifiIcon(int color);
    void displayLocalTime();
    void displayAirconStatus(controlInfo ci);
    void displayLocalTime(tm timeinfo);
    void displayZoneStatus(zonesStatusStruct zns);
  private:
    Adafruit_ILI9341 *_tft;
};

#endif
