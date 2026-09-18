#include <Arduino.h>
#include "GlobalModule.h"

void showPage24() {
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextDatum(MC_DATUM);
  tft.setTextSize(2);
  tft.drawString("This page is unavailable", tft.width() / 2, tft.height() / 2 - 12);
  tft.setTextSize(1);
  tft.drawString("Use the coin display pages", tft.width() / 2, tft.height() / 2 + 18);
}
