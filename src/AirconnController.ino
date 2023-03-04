// Learn about the ESP32 WiFi simulation in
// https://docs.wokwi.com/guides/esp32-wifi

// MOSI: 23
// MISO: 19
// SCK: 18
// SS: 5
#if defined(ARDUINO_ARCH_ESP32)
#define TFT_CS     22
#define TFT_DC     21
#include <WiFi.h>
#include <WebServer.h>
#include <HTTPClient.h>
#include <ESPmDNS.h>
#define FORMAT_ON_FAIL  true
using WebServerClass = WebServer;
#elif defined(ARDUINO_ARCH_ESP8266)
#define TFT_CS     22
#define TFT_DC     21
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266mDNS.h>
#define FORMAT_ON_FAIL
using WebServerClass = ESP8266WebServer;

#else
  #error Unsupported board selection.
#endif

#include <AutoConnect.h>
#include <ArduinoJson.h>

#include <LittleFS.h>
FS& FlashFS = LittleFS;

#define PARAM_FILE      "/aircon_definition.json"

#include <OneButton.h>

#include <Wire.h>
#include <AsyncDelay.h>

#include "AirconControls.h"
#include "AirconControlsHTTP.cpp"

#include "DisplayController.h"
#include "DisplayControllerTFTString.cpp"
#include "DisplayControllerSerial.cpp"

#include "SPI.h"
#include "Adafruit_GFX.h"
#include "Adafruit_ILI9341.h"

/*
   #define TFT_CS    D2          // TFT CS  pin is connected to NodeMCU pin D2
   #define TFT_RST   D3          // TFT RST pin is connected to NodeMCU pin D3
   #define TFT_DC    D4   

*/

WebServerClass  server;
AutoConnect portal(server);
AutoConnectConfig config;
AutoConnectAux  elementsAux;
AutoConnectAux  saveAux;

String airconURL;

Adafruit_ILI9341 display = Adafruit_ILI9341(TFT_CS, TFT_DC, 3);

AirconControls*   airconControls = new AirconControlsHTTP(airconURL);

#ifdef SERIAL_SCREEN
  DisplayController* displayController = new DisplayControllerSerial();
#else
  DisplayController* displayController = new DisplayControllerTFTString(&display);
#endif


AsyncDelay delay_5s;
AsyncDelay delay_60s;

#define NTP_SERVER     "pool.ntp.org"
#define UTC_OFFSET     10 * 60 * 60
#define UTC_OFFSET_DST 0

int myZoneId = -1;


#define BUTTON_POWER_STATUS 21

OneButton powerStatusButton = OneButton(
  BUTTON_POWER_STATUS,  // Input pin for the button
  true,        // Button is active LOW
  true         // Enable internal pull-up resistor
);

zonesStatusStruct zS;
controlInfo ci;

static const char PAGE_ELEMENTS[] PROGMEM = R"(
{
  "uri": "/elements",
  "title": "Aircon Definitions",
  "menu": true,
  "element": [
    {
      "name": "tablecss",
      "type": "ACStyle",
      "value": "table{font-family:arial,sans-serif;border-collapse:collapse;width:100%;color:black;}td,th{border:1px solid #dddddd;text-align:center;padding:8px;}tr:nth-child(even){background-color:#dddddd;}"
    },
    {
      "name": "text",
      "type": "ACText",
      "value": "Aircon configuration",
      "style": "font-family:Arial;font-size:18px;font-weight:400;color:#191970",
      "posterior": "div"
    },
    {
      "name": "airconIP",
      "type": "ACInput",
      "label": "Aircon IP",
      "placeholder": "Please provide your aircon IP address",
      "pattern": "^(([a-zA-Z0-9]|[a-zA-Z0-9][a-zA-Z0-9\\-]*[a-zA-Z0-9])\\.)*([A-Za-z0-9]|[A-Za-z0-9][A-Za-z0-9\\-]*[A-Za-z0-9])$"
    },
    {
      "name": "zoneId",
      "type": "ACInput",
      "label": "Zone ID",
      "value": "7",
      "apply": "number",
      "pattern": "\\d*"
    },
    {
      "name": "save",
      "type": "ACSubmit",
      "value": "Save",
      "uri": "/save"
    },
    {
      "name": "adjust_width",
      "type": "ACElement",
      "value": "<script type=\"text/javascript\">window.onload=function(){var t=document.querySelectorAll(\"input[type='text']\");for(i=0;i<t.length;i++){var e=t[i].getAttribute(\"placeholder\");e&&t[i].setAttribute(\"size\",e.length*.8)}};</script>"
    }
  ]
}
)";

static const char PAGE_SAVE[] PROGMEM = R"(
{
  "uri": "/save",
  "title": "Aircon Definitions",
  "menu": false,
  "element": [
    {
      "name": "caption",
      "type": "ACText",
      "format": "Aircon Configuration has been saved",
      "style": "font-family:Arial;font-size:18px;font-weight:400;color:#191970"
    },
    {
      "name": "validated",
      "type": "ACText",
      "style": "color:red",
      "posterior": "div"
    },
    {
      "name": "echo",
      "type": "ACText",
      "style": "font-family:monospace;font-size:small;white-space:pre;",
      "posterior": "div"
    },
    {
      "name": "ok",
      "type": "ACSubmit",
      "value": "OK",
      "uri": "/elements"
    }
  ]
}
)";

void parse_aircon_config_from_string(String value){

      StaticJsonDocument<600> doc;
      DeserializationError error = deserializeJson(doc, value);
      if (error) {
        Serial.println("JSON de-serialization failed: " + String(error.c_str()));
      }
      else {
        airconURL = "http://";
        airconURL.concat(doc[0]["value"].as<String>());
        Serial.print(" IP JSON:");
        Serial.print(airconURL);
        Serial.println("<-");

        myZoneId = doc[1]["value"].as<int>();
        Serial.print(" ZONE JSON:");
        Serial.print(myZoneId);
        Serial.println("<-");
      }
}
void loadAirconConfig(){

  File param = FlashFS.open(PARAM_FILE, "r");
  if (param) {
    parse_aircon_config_from_string(param.readString());
    Serial.print(" set new IP:");
    Serial.println(airconURL);
    airconControls->setURL(airconURL);
    param.close();
  }
  else {
    Serial.println("Filesystem failed to open.");
  }
}


void powerButtonChanged(const int buttonState){
  Serial.print("powerButtonChanged: ");
  Serial.println(buttonState);

  // verify the aircon status
  // if it's ON we need to switch the status of this zone
  // if it's OFF we need to switch all the other zones OFF and this one ON
  if(buttonState == 1){
    if(ci.power == POWER_ON){
      Serial.println("Aircon is ON"); 
      if(zS.zoneStatus[myZoneId] == POWER_ON){
        Serial.print("Setting OFF to zone:"); 
        Serial.println(myZoneId); 
        zS.zoneStatus[myZoneId]=POWER_OFF;
        // if no other zone is active, shut it down
        if(!airconControls->hasActiveZone(zS)){
          Serial.println("Turning Power OFF"); 
          airconControls->setPowerStatus(ci, POWER_OFF);
        }else{
          Serial.println("Keeping Power ON"); 
        }
      }else{
        Serial.print("Setting ON to zone:"); 
        Serial.println(myZoneId); 
        zS.zoneStatus[myZoneId]=POWER_ON;
      }
    }else{
      Serial.println("Aircon is off, resetting all zones");
      for(int i = 0; i < numberOfZones; i++)       {
          zS.zoneStatus[i]=POWER_OFF;
      }
      zS.zoneStatus[myZoneId]=POWER_ON;
      airconControls->setPowerStatus(ci, POWER_ON);
    }
    Serial.print("Setting My zone status to:");
    Serial.println(zS.zoneStatus[myZoneId]);
    airconControls->setZoneStatus(zS);
  }
}

void displayWifiStatus(){
  if(WiFi.status() == WL_CONNECTED) {
    displayController->displayWifiIcon(ILI9341_GREEN);
  }else{
    displayController->displayWifiIcon(ILI9341_WHITE);
  }
}
void config_wifi_portal(){
    FlashFS.begin(FORMAT_ON_FAIL);

  // Responder of root page handled directly from WebServer class.
  server.on("/", []() {
    String content = "Place the root page with the sketch application.&ensp;";
    content += AUTOCONNECT_LINK(COG_24);
    server.send(200, "text/html", content);
  });

  // Load a custom web page described in JSON as PAGE_ELEMENT and
  // register a handler. This handler will be invoked from
  // AutoConnectSubmit named the Load defined on the same page.
  elementsAux.load(FPSTR(PAGE_ELEMENTS));
  elementsAux.on([] (AutoConnectAux& aux, PageArgument& arg) {
    Serial.print("Accessin elementsAux.on ");
    Serial.println(portal.where());
    if (portal.where() == "/elements" or portal.where() == "") {
      // Use the AutoConnect::where function to identify the referer.
      // Since this handler only supports AutoConnectSubmit called the
      // Load, it uses the uri of the custom web page placed to
      // determine whether the Load was called me or not.
      File param = FlashFS.open(PARAM_FILE, "r");
      if (param) {
        Serial.println("LOADING INITIAL AIRCON SETTINGS");
        String s = param.readString();
        Serial.println(s);
        aux.loadElement(s, { "airconIP", "zoneId" } );
        param.close();
      }else{
        Serial.println("COULDN'T OPEN FILE TO LOAD INITIAL AIRCON SETTINGS");
      }
    }
    return String();
  });

  saveAux.load(FPSTR(PAGE_SAVE));
  saveAux.on([] (AutoConnectAux& aux, PageArgument& arg) {
    // You can validate input values before saving with
    // AutoConnectInput::isValid function.
    // Verification is using performed regular expression set in the
    // pattern attribute in advance.
    AutoConnectInput& input = elementsAux["airconIP"].as<AutoConnectInput>();
    aux["validated"].value = input.isValid() ? String() : String("Input data pattern missmatched.");

    // The following line sets only the value, but it is HTMLified as
    // formatted text using the format attribute.
    aux["caption"].value = PARAM_FILE;

    File param = FlashFS.open(PARAM_FILE, "w");
    if (param) {
      // Save as a loadable set for parameters.
      elementsAux.saveElement(param, { "airconIP", "zoneId"});
      param.close();
      // Read the saved elements again to display.
      param = FlashFS.open(PARAM_FILE, "r");
      aux["echo"].value = param.readString();
      param.close();
      parse_aircon_config_from_string(aux["echo"].value);
    }
    else {
      aux["echo"].value = "Filesystem failed to open.";
    }
    return String();
  });
  
  portal.join({ elementsAux, saveAux });
  
  config.apid = "AirconController_123";
  config.psk = "0234567890";

  portal.config(config);
  portal.begin();

}
void setup() {

  Serial.begin(115200);
  Serial.println("setup 1");

  displayController->startDisplay();

  config_wifi_portal();
  delay_5s.start(5000, AsyncDelay::MILLIS);
  delay_60s.start(60000, AsyncDelay::MILLIS);

  // powerStatusButton.setCallback(powerButtonChanged);
  powerStatusButton.attachClick([]() {
    Serial.println("Cliked Pressed!");
  }); 

  powerStatusButton.attachDoubleClick([]() {
    Serial.println("Double Pressed!");
  });
  powerStatusButton.setPressTicks(3000);
  powerStatusButton.attachLongPressStop([]() {
    Serial.println("Long Pressed!");
  });

  loadAirconConfig();

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.print("IP address:");
  Serial.print(WiFi.localIP());
  Serial.println("<-");

  configTime(UTC_OFFSET, UTC_OFFSET_DST, NTP_SERVER);
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Couldn't get the time from server");
  }
  
}

void loop() {

  portal.handleClient();
  powerStatusButton.tick();
  
  if (delay_5s.isExpired()) {
    delay_5s.repeat(); // Count from when the delay expired, not now

    displayWifiStatus();
    
    if(airconURL.isEmpty() or myZoneId==-1){
      Serial.println("Aircon Config not defined");
      displayController->airconNotFound();
    }else{
      controlInfo ci = airconControls->readControlInfo();
      if(ci.power == "-1"){
        Serial.println("Aircon Config is not correct");
        displayController->airconNotFound();
      }else{
        displayController->displayAirconStatus(ci);

        zonesStatusStruct zns= airconControls->readZoneStatus();
        displayController->displayZoneStatus(zns);
        }
    }
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo)) {
      Serial.println("Connection Err");
    }else{
      displayController->displayLocalTime(timeinfo);
    }
    Serial.println("");
    Serial.println("");
    Serial.println("");
  }

  if(delay_60s.isExpired()){
    delay_60s.repeat(); // Count from when the delay expired, not now
  }

}
