#ifndef PAGE25_H
#define PAGE25_H

#include <TFT_eSPI.h>
#include <LittleFS.h>
#include <ArduinoJson.h>



void checkInversion(TFT_eSPI &tft);
void saveInversion(bool inv);
#endif
