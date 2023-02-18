#include "Aircon.h"
#include <Arduino.h>
#include <WiFiClient.h>

#if defined(ESP32)
#include <HTTPClient.h>
#elif defined(ESP8266)
#include <ESP8266HTTPClient.h>
#else
  #error Unsupported board selection.
#endif

// temporary until we connect to wifi
String powerStatus = OFF;

controlInfo parseControlInfo(String input) {
  controlInfo result;

  int commaIndex = input.indexOf(',');
  while (commaIndex != -1) {
    int equalsIndex = input.indexOf('=');
    String field = input.substring(0, equalsIndex);
    if (field == "ret") {
      result.ret = input.substring(equalsIndex + 1, commaIndex);
    } else if (field == "pow") {
      result.power = input.substring(equalsIndex + 1, commaIndex);
    } else if (field == "mode") {
      result.mode = input.substring(equalsIndex + 1, commaIndex);
    } else if (field == "stemp") {
      result.temperature = input.substring(equalsIndex + 1, commaIndex);
    } else if (field == "f_rate") {
      result.fanRate = input.substring(equalsIndex + 1, commaIndex);
    } else if (field == "f_dir") {
      result.f_dir = input.substring(equalsIndex + 1, commaIndex);
    }
    input = input.substring(commaIndex + 1);
    commaIndex = input.indexOf(',');
  }

  return result;
}

String generateControlInfoString(controlInfo v) {
  String result = "pow=" + v.power + ",mode=" + v.mode + ",stemp=" + v.temperature + ",f_rate=" + v.fanRate + ",f_dir="+v.f_dir;
  return result;
}

unsigned char h2int(char c)
{
    if (c >= '0' && c <='9'){
        return((unsigned char)c - '0');
    }
    if (c >= 'a' && c <='f'){
        return((unsigned char)c - 'a' + 10);
    }
    if (c >= 'A' && c <='F'){
        return((unsigned char)c - 'A' + 10);
    }
    return(0);
}

String urlDecode(String str)
{
    String encodedString="";
    char c;
    char code0;
    char code1;
    for (int i =0; i < str.length(); i++){
        c=str.charAt(i);
      if (c == '+'){
        encodedString+=' ';  
      }else if (c == '%') {
        i++;
        code0=str.charAt(i);
        i++;
        code1=str.charAt(i);
        c = (h2int(code0) << 4) | h2int(code1);
        encodedString+=c;
      } else{
        
        encodedString+=c;  
      }
      
      yield();
    }
    
   return encodedString;
}

void split(String inputString, char separator, String *outputArray, int size) {
  int count = 0;
  int lastIndex = 0;
  for (int i = 0; i < inputString.length(); i++) {
    if (inputString[i] == separator) {
      outputArray[count++] = inputString.substring(lastIndex, i);
      lastIndex = i + 1;
    }
  }
  outputArray[count++] = inputString.substring(lastIndex);
}

void parseZoneOneOff(zonesStatusStruct &zonesStatus, String input){
  String zoneStatusArray[8];
  split(input, ';', zoneStatusArray, 8);

  String zoneName[8];
  split(input, ';', zoneName, 8);


  for (int i = 0; i < 8; i++) {
    zonesStatus.zoneStatus[i] = zoneStatusArray[i];
    zonesStatus.zoneNames[i] = zoneName[i];
  }
}

void splitZoneStatus(zonesStatusStruct &zns, String inputString){
  // Split the string into parts separated by commas
  int partCount = 3;
  String parts[partCount];
  split(inputString, ',', parts, partCount);

  // Loop through each part
  for (int i = 0; i < partCount; i++) {
    // Split each part into key/value pairs
    String keyValue[2];
    split(parts[i], '=', keyValue, 2);
    String key = keyValue[0];
    String value = urlDecode(keyValue[1]);
    if(key=="zone_onoff"){
      parseZoneOneOff(zns, value);
      return;
    }
  }
}

zonesStatusStruct readZoneStatus(){

  HTTPClient http;

  String serverPath =  "http://192.168.1.4/skyfi/aircon/get_zone_setting";
  
  WiFiClient client;
  // Your Domain name with URL path or IP address with path
  http.begin(client, serverPath.c_str());
  
  // If you need Node-RED/server authentication, insert user and password below
  //http.setAuthorization("REPLACE_WITH_SERVER_USERNAME", "REPLACE_WITH_SERVER_PASSWORD");
  
  zonesStatusStruct zns;

  // Send HTTP GET request
  int httpResponseCode = http.GET();
  
  if (httpResponseCode>0) {
    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);
    String payload = http.getString();
    splitZoneStatus(zns, urlDecode(payload));
  } else {
    Serial.print("Error code: ");
    Serial.println(httpResponseCode);
  }
  // Free resources
  http.end();
  return zns;
}

bool hasActiveZone(zonesStatusStruct zns){
  for(int i = 0; i < numberOfZones; i++) {
    if(zns.zoneStatus[i] == ON){
      Serial.println("Some active zones."); 
      return true;
    }
  }
  Serial.println("No more active zones"); 
  return false;
}

String sendControlInfo(controlInfo v){
  HTTPClient http;
  
  String response = "";
  
  String queryString = generateControlInfoString(v);

  WiFiClient client;

  http.begin(client, "http://192.168.0.100/post?"+queryString);

  http.addHeader("Content-Type", "text/plain");
  int httpCode = http.POST("");

  if (httpCode > 0) {
    Serial.println(httpCode);
    response = http.getString();
  } else {
    Serial.println("Error on HTTP request");
  }

  http.end();

  return response;
}

controlInfo retrieveControlInfo(){

  HTTPClient http;

  String serverPath =  "http://192.168.1.4/skyfi/aircon/get_control_info";
  
  WiFiClient client;
  // Your Domain name with URL path or IP address with path
  http.begin(client, serverPath.c_str());
  
  // If you need Node-RED/server authentication, insert user and password below
  //http.setAuthorization("REPLACE_WITH_SERVER_USERNAME", "REPLACE_WITH_SERVER_PASSWORD");
  
  controlInfo ci;

  // Send HTTP GET request
  int httpResponseCode = http.GET();
  
  if (httpResponseCode>0) {
    //Serial.print("HTTP Response code: ");
    //Serial.println(httpResponseCode);
    String payload = http.getString();
    ci=parseControlInfo(payload);
  } else {
    Serial.print("Error code: ");
    Serial.println(httpResponseCode);
  }
  // Free resources
  http.end();

  return ci;
}

void setPowerStatus(controlInfo controlInfo, String status){
  
  controlInfo.power = status;

  //sendControlInfo(controlInfo);

  //the lines below can go away when the HTTP in properly implemented
  Serial.print("Setting POWER status to:");
  Serial.println(status);
  powerStatus = status;
}

void setZoneStatus(zonesStatusStruct zns){
  String zoneStatusString;
  String zoneNamesString;

  for (int i = 0; i < numberOfZones; i++) {
    if (i > 0) {
      zoneStatusString.concat(";");
      zoneNamesString.concat(";");
    }
    zoneStatusString.concat(zns.zoneStatus[i]);
    zoneNamesString.concat(zns.zoneStatus[i]);
  }

  HTTPClient http;
  
  String response = "";

  String queryString = "zone_onoff="+zoneStatusString+"&zone_name="+zoneNamesString;

  Serial.println(queryString);
  WiFiClient client;

  http.begin(client, "http://192.168.1.4/skyfi/aircon/set_zone_setting?" + queryString );
  http.addHeader("Content-Type", "text/plain");
  int httpCode = http.POST("");

  if (httpCode > 0) {
    Serial.println(httpCode);
    response = http.getString();
    Serial.println("Response from set Zone");
  } else {
    Serial.println("Error on HTTP request");
  }

  http.end();

}