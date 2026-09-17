#pragma once
#include <Arduino.h>
#include <TFT_eSPI.h>

bool drawSdJpeg(TFT_eSPI& screen, const char* filename, int x, int y);
