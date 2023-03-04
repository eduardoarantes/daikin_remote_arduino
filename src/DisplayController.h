#ifndef DisplayController_h
#define DisplayController_h

#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include "AirconControls.h"

class DisplayController {
  public:
    virtual void startDisplay() = 0;
    virtual void displayWifiIcon(int color) = 0;
    virtual void displayAirconStatus(controlInfo ci) = 0;
    virtual void displayLocalTime(tm timeinfo) = 0;
    virtual void displayZoneStatus(zonesStatusStruct zns) = 0;
    virtual void airconNotFound() = 0;

};

#endif
