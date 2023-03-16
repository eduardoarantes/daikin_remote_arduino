# daikin_remote_arduino
I creates a remote control for Daikin aircon with skyfi

# Motivation

I have a multi-zone air conditioner and only one control.
I understand that we can control with the mobile app, but I don't expect guests to need to install the app.

# It is planned to have 2 modes

## Single Zone Control

* on/off only – It will live in each one of the bedrooms and the only function is to turn on and off the aircon in that zone
  * when turning it off, it will turn the unit off if that was the only zone in operation
  *  when turning it on, and the unit is off, it will turn off all other zones, turn on the current one and turn the unit on

## Master control

* it will control the mode (cool, heat, ...), temperature and zones. Pretty much everything.


# Arduino

It aims to work with Arduino ESP32 and ESP8266, but it shouldn't be hard to work with any other with wifi connection.


# Screens

It has option to work with 
* OLED for the single zone controls
* TFT touch for master mode

# Operation

The control leverage the AutoConnect library to expose a wifi AP that can be used to define which wifi network your controler is, the controller IP and the zone ID.

