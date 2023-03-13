#include <Arduino.h>

#pragma once

struct controlInfo {
  String ret;
  String power;
  String mode; //0 - fan. 1 - heat 2 - cool 3 - auto 7 - dry
  String temperature;
  String fanRate; // 1 - level 1  
                  // 3 - level 2   
                  // 5 - level 3
  String shum;
  String f_dir;
};

const int MODE_FAN = 0;
const int MODE_HEAT = 1;
const int MODE_COLD = 2;
const int MODE_AUTO = 3;
const int MODE_DRY = 7;


const int numberOfZones = 8;

const String POWER_ON = "1";
const String POWER_OFF = "0";

struct zonesStatusStruct{
  String zoneStatus[8];
  String zoneNames[8];
};

class AirconControls {
  public:
    virtual zonesStatusStruct readZoneStatus();
    virtual bool hasActiveZone(zonesStatusStruct zS);
    virtual String sendControlInfo(controlInfo v);
    virtual controlInfo readControlInfo();
    virtual void setPowerStatus(controlInfo ci, String status);
    virtual void setZoneStatus(zonesStatusStruct zS);
    virtual void setURL(String url);
    virtual String getURL();
};