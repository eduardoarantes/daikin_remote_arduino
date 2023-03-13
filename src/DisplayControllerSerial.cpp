#include "DisplayController.h"
#include "AirconControls.h"

class DisplayControllerSerial: public DisplayController {
  
public: 
  DisplayControllerSerial(){
  }

  void startDisplay(){
    
  }

  void displayWifiIcon(int status){
    Serial.println(" Wifi status: "+status);
  }

  void displayLocalTime(tm timeinfo) {
    byte h, m, s;
    char str[30];
    long thetime;
    thetime = millis();
    h = (thetime / 3600000) / 1000;
    m = (thetime / 1000 - (3600 * h)) / 60;
    s = (thetime / 1000 - (3600 * h) - (m * 60));
    sprintf(str, "Time: %02d:%02d:%02d", h, m, s);

    Serial.println(str);
  }

  void displayAirconStatus(controlInfo ci){
      
      Serial.print("Aircon status: ");
      Serial.println(ci.power);
      
      Serial.print("Aircon temp: ");
      Serial.println(ci.temperature);

      Serial.print("Aircon Mode: ");
      
      int mode = ci.mode.toInt();
      switch (mode)
      {
      case 0:
        Serial.println("Fan");
        break;
      case 1:
        Serial.println("Heat");
        break;
      case 2:
        Serial.println("Cold");
        break;
      case 3:
        Serial.println("Auto");
        break;
      case 7:
        Serial.println("Dry");
        break;
      default:
        Serial.println("Undefined");
        break;
      }
  }

  void displayZoneStatus(zonesStatusStruct zns){
      int printZoneCount=0;
      for (size_t i = 0; i < numberOfZones; i++)
      {
        if(zns.zoneNames[i]=="empty"){
          continue;
        }
        Serial.print(zns.zoneNames[i]+" : ");
        Serial.println(zns.zoneStatus[i]);
        printZoneCount++;
      }
  }
  void airconNotFound(){
    Serial.println("Aircon connection failed");
  }
};