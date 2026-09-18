#include <TFT_eSPI.h> // Include the graphics library (this includes the sprite functions)
#include <Arduino.h>
#include "GlobalModule.h"
#include "GlobalParameter.h"
#include "GlobalFunction.h"
#include "page2.h"

// Adjusted pastel colors for better harmony
#define PASTEL_BG       tft.color565(240, 240, 240) // Light pastel background

void showPage13() {
  // Clear the screen and set background to black
  tft.fillScreen(TFT_BLACK);

  // Display Left Arrow with the "left" direction
  drawArrow(30, 10, "left");

  // Display the "OTA" title
  tft.setTextSize(3);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextDatum(ML_DATUM);
  tft.drawString("OTA", tft.width() / 2 - 20, 25); // Adjust the position as needed

  // Display the current and next version text
  tft.setTextSize(2);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextDatum(ML_DATUM);
  tft.drawString("Your Current Version: 1.0", 10, 80);
  tft.drawString("Next Version: 2.0", 10, 110);

  // Draw the "Detail" button
  drawGradientRoundedRect(10, 175, 110, 60, 10, tft.color565(173, 216, 230), tft.color565(135, 206, 250));
  tft.setTextColor(TFT_WHITE);
  tft.drawString("Detail", 30, 205); // Centered text in the button

  // Draw the "Upgrade" button
  drawGradientRoundedRect(200, 175, 110, 60, 10, tft.color565(144, 238, 144), tft.color565(60, 179, 113));
  tft.setTextColor(TFT_WHITE);
  tft.drawString("Upgrade", 213, 205); // Centered text in the button
}
