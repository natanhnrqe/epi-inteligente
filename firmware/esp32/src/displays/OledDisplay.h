#ifndef OLEDDISPLAY_H
#define OLEDDISPLAY_H

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include  "../models/EmergencyProfile.h"

class OledDisplay {
    private:
        TwoWire& bus;
        Adafruit_SSD1306 display;

    public:
        explicit OledDisplay(TwoWire& bus);
        bool begin(int sdaPin, int sclPin);
        void showBattery(int percent);
        void showEmergencyPage(const EmergencyProfile& profile, int page);
};

#endif
