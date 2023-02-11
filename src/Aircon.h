#include <Arduino.h>
#ifndef Aircon_h
#define Aircon_h

struct controlInfo {
  String ret;
  String power;
  String mode; //0 - fan. 1 - heat 2 - cool 3 - auto 7 - dry
  String temperature;
  String fanRate; // 1 - level 1  3 - level 2   5 - level 3
  String shum;
  String f_dir;
};

const int numberOfZones = 8;
const String zoneNamesString = "UPSTAIRS;MASTER;MEL;empty;DOWNSTAIRS;GEST1;GEST2;EDU";

const String zoneNamesArray[] = {"UPSTAIRS", "MASTER", "MEL", "empty", "DOWNSTAIRS", "GEST1", "GEST2", "EDU"} ;

const String ON = "1";
const String OFF = "0";

struct zonesStatusStruct{
  String zoneStatus[8];
};

controlInfo parseControlInfo(String input);

String generateControlInfoString(controlInfo v);

zonesStatusStruct readZoneStatus();

bool hasActiveZone(zonesStatusStruct zS);

String sendControlInfo(controlInfo v);

controlInfo retrieveControlInfo();

void setPowerStatus(controlInfo ci, String status);

void setZoneStatus(zonesStatusStruct zS);

#endif