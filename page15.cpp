#include <TFT_eSPI.h> // Include the graphics library (this includes the sprite functions)
#include <Arduino.h>
#include "GlobalModule.h"
#include "GlobalParameter.h"
#include "GlobalFunction.h"
#include "JpgSupport.h"
#include "JPEGDecoder.h"

// Adjusted pastel colors for better harmony
#define PASTEL_BG       tft.color565(240, 240, 240) // Light pastel background

void showPage15() {
  // Clear the screen and set background to black
  tft.fillScreen(TFT_BLACK);

  // Display Left Arrow with the "left" direction
  drawArrow(30, 10, "left");

  // Display the "LINKS" title
  tft.setTextSize(3);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextDatum(ML_DATUM);
  tft.drawString("PAYMENT", tft.width() / 2 - 37, 25); // Adjust the position as needed

  // Display the current and next version text
  tft.setTextSize(2);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextDatum(ML_DATUM);
  tft.drawString("Your CCP: 1000000", 20, 70);
  tft.drawString("Use CCP: 99", 20, 110);

  drawGradientRoundedRect(210, 90, 100, 40, 10, tft.color565(173, 216, 230), tft.color565(135, 206, 250));
  tft.setTextColor(TFT_WHITE);
   tft.setTextSize(2);
  tft.drawString("/MONTH", 232, 110);

  // Draw the "Detail" button
  drawGradientRoundedRect(10, 175, 110, 60, 10, tft.color565(255, 182, 193), tft.color565(255, 105, 180));
  tft.setTextColor(TFT_WHITE);
  tft.drawString("TOP UP", 32, 205); // Centered text in the button

  // Draw the "Upgrade" button
  drawGradientRoundedRect(200, 175, 110, 60, 10, tft.color565(144, 238, 144), tft.color565(60, 179, 113));
  tft.setTextColor(TFT_WHITE);
  tft.drawString("CONFIRM", 213, 205); // Centered text in the button
}
