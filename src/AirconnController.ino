// Learn about the ESP32 WiFi simulation in
// https://docs.wokwi.com/guides/esp32-wifi

// MOSI: 23
// MISO: 19
// SCK: 18
// SS: 5

#include <WiFi.h>
#include <HTTPClient.h>

#include <Wire.h>
#include <AsyncDelay.h>

//https://github.com/maykon/ButtonDebounce
#include <ButtonDebounce.h>

#include "Aircon.h"
#include "DisplayController.h"

#include "SPI.h"
#include "Adafruit_GFX.h"
#include "Adafruit_ILI9341.h"

#define TFT_DC 25
#define TFT_CS 5
Adafruit_ILI9341 display = Adafruit_ILI9341(TFT_CS, TFT_DC);

DisplayController displayController = DisplayController(&display);

AsyncDelay delay_5s;
AsyncDelay delay_60s;

#define NTP_SERVER     "pool.ntp.org"
#define UTC_OFFSET     10 * 60 * 60
#define UTC_OFFSET_DST 0

#define AIRCON_IP "192.168.1.4"

#define BUTTON_POWER_STATUS 21

ButtonDebounce powerStatusButton(BUTTON_POWER_STATUS, 300);

int myZoneId = 7;

const int rgbPins[] = { 14, 12, 13};  // PWM pins for Red, Green and Blue colors of RGB led.

void powerButtonChanged(const int buttonState){
  Serial.print("powerButtonChanged: ");
  Serial.println(buttonState);

  // verify the aircon status
  // if it's ON we need to switch the status of this zone
  // if it's OFF we need to switch all the other zones OFF and this one ON
  if(buttonState == 1){
  
    zonesStatusStruct zS = readZoneStatus();
    
    controlInfo ci = retrieveControlInfo();

    if(ci.power == ON){
      Serial.println("Aircon is ON"); 
      if(zS.zoneStatus[myZoneId] == ON){
        Serial.print("Setting OFF to zone:"); 
        Serial.println(myZoneId); 
        zS.zoneStatus[myZoneId]=OFF;
        // if no other zone is active, shut it down
        if(!hasActiveZone(zS)){
          Serial.println("Turning Power OFF"); 
          setPowerStatus(ci, OFF);
        }else{
          Serial.println("Keeping Power ON"); 
        }
      }else{
        Serial.print("Setting ON to zone:"); 
        Serial.println(myZoneId); 
        zS.zoneStatus[myZoneId]=ON;
      }
    }else{
      Serial.println("Aircon is off, resetting all zones");
      for(int i = 0; i < numberOfZones; i++)       {
          zS.zoneStatus[i]=OFF;
      }
      zS.zoneStatus[myZoneId]=ON;
      setPowerStatus(ci, ON);
    }
    Serial.print("Setting My zone status to:");
    Serial.println(zS.zoneStatus[myZoneId]);
    setZoneStatus(zS);
  }
}


void displayWifiStatus(){
  if(WiFi.status() == WL_CONNECTED) {
    displayController.drawWifiIcon(ILI9341_GREEN);
  }else{
    displayController.drawWifiIcon(ILI9341_WHITE);
  }
}

WiFiServer server(80);

// Replace with your network credentials
const char* ssid     = "ESP32-Access-Point";
const char* password = "123456789";

void ap_init()
{
  WiFi.softAP(ssid, password);

  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(IP);
  
  server.begin();
}

void setup() {

  WiFi.begin("Wokwi-GUEST", "", 6);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.println("trying to connect to GUEST");
    displayController.drawWifiIcon(ILI9341_ORANGE);
    delay(250);
    displayController.drawWifiIcon(ILI9341_WHITE);
  }

  displayController.drawWifiIcon(ILI9341_GREEN);

  delay_5s.start(5000, AsyncDelay::MILLIS);
  delay_60s.start(60000, AsyncDelay::MILLIS);

  display.begin();

  powerStatusButton.setCallback(powerButtonChanged);

  controlInfo ci = retrieveControlInfo();

  displayController.displayAirconStatus(ci.power, ci.temperature);
  displayWifiStatus();

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  configTime(UTC_OFFSET, UTC_OFFSET_DST, NTP_SERVER);
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Connection Err");
    return;
  }
  displayController.printLocalTime(timeinfo);
  
}
// Variable to store the HTTP request
String header;

void enableAPClient(){
   WiFiClient client = server.available();   // Listen for incoming clients

  if (client) {                             // If a new client connects,
    Serial.println("New Client.");          // print a message out in the serial port
    String currentLine = "";                // make a String to hold incoming data from the client
    while (client.connected()) {            // loop while the client's connected
      if (client.available()) {             // if there's bytes to read from the client,
        char c = client.read();             // read a byte, then
        Serial.write(c);                    // print it out the serial monitor
        header += c;
        if (c == '\n') {                    // if the byte is a newline character
          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) {
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();
            
            // turns the GPIOs on and off
            if (header.indexOf("GET /26/on") >= 0) {
              Serial.println("GPIO 26 on");
            } else if (header.indexOf("GET /26/off") >= 0) {
              Serial.println("GPIO 26 off");
            } else if (header.indexOf("GET /27/on") >= 0) {
              Serial.println("GPIO 27 on");
            } else if (header.indexOf("GET /27/off") >= 0) {
              Serial.println("GPIO 27 off");
            }
            
            // Display the HTML web page
            client.println("<!DOCTYPE html><html>");
            client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
            client.println("<link rel=\"icon\" href=\"data:,\">");
            // CSS to style the on/off buttons 
            // Feel free to change the background-color and font-size attributes to fit your preferences
            client.println("<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}");
            client.println(".button { background-color: #4CAF50; border: none; color: white; padding: 16px 40px;");
            client.println("text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}");
            client.println(".button2 {background-color: #555555;}</style></head>");
            
            // Web Page Heading
            client.println("<body><h1>ESP32 Web Server</h1>");
            
            // Display current state, and ON/OFF buttons for GPIO 26  
            client.println("<p>GPIO 26 - State </p>");
              client.println("<p><a href=\"/26/off\"><button class=\"button button2\">OFF</button></a></p>");
               
            // Display current state, and ON/OFF buttons for GPIO 27  
            client.println("<p>GPIO 27 - State </p>");
            // If the output27State is off, it displays the ON button       
              client.println("<p><a href=\"/27/off\"><button class=\"button button2\">OFF</button></a></p>");
            client.println("</body></html>");
            
            // The HTTP response ends with another blank line
            client.println();
            // Break out of the while loop
            break;
          } else { // if you got a newline, then clear currentLine
            currentLine = "";
          }
        } else if (c != '\r') {  // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
        }
      }
    }
    // Clear the header variable
    header = "";
    // Close the connection
    client.stop();
    Serial.println("Client disconnected.");
    Serial.println("");
  }
}

void loop() {

  powerStatusButton.update();
  
  if (delay_5s.isExpired()) {
    displayWifiStatus();
    controlInfo ci = retrieveControlInfo();
    displayController.displayAirconStatus(ci.power, ci.temperature);
    delay_5s.repeat(); // Count from when the delay expired, not now
    Serial.println("5s");
  }

  if(delay_60s.isExpired()){
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo)) {
      Serial.println("Connection Err");
      return;
    }
    displayController.printLocalTime(timeinfo);
    delay_60s.repeat(); // Count from when the delay expired, not now
  }

}
