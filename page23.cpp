#include <TFT_eSPI.h> // Include the graphics library (this includes the sprite functions)
#include <Arduino.h>
#include "GlobalModule.h"
#include "GlobalParameter.h"
#include "GlobalFunction.h"
#include "JpgSupport.h"
#include "JPEGDecoder.h"

// Adjusted pastel colors for better harmony
#define PASTEL_BG       tft.color565(240, 240, 240) // Light pastel background

void showPage23() {
  // Clear the screen and set background to black
  tft.fillScreen(TFT_BLACK);


  // Display the "LINKS" title
  tft.setTextSize(2);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextDatum(ML_DATUM);
  tft.drawString("Under Construction", tft.width() / 2 - 82, 28); // Adjust the position as needed

 // Draw Left Arrow
  drawArrow(30, 10, "left");
  
  // Display the current and next version text
  tft.setTextSize(4);
  tft.setTextColor(TFT_RED, TFT_BLACK);
  tft.setTextDatum(ML_DATUM);
  tft.drawString("Coming Soon ", 27, 120);
  
}
