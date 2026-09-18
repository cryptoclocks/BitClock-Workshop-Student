#include <TFT_eSPI.h> // Include the graphics library (this includes the sprite functions)
#include <Arduino.h>
#include "GlobalModule.h"
#include "GlobalParameter.h"
#include "GlobalFunction.h"
#include "JpgSupport.h"
#include "JPEGDecoder.h"

// Adjusted pastel colors for better harmony
#define PASTEL_BG       tft.color565(240, 240, 240) // Light pastel background

void showPage14() {
  // Clear the screen and set background to black
  tft.fillScreen(TFT_BLACK);

  // Display Left Arrow with the "left" direction
  drawArrow(30, 10, "left");

  // Display the "LINKS" title
  tft.setTextSize(3);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextDatum(ML_DATUM);
  tft.drawString("LINKS", tft.width() / 2 - 25, 25); // Adjust the position as needed
  String path = "/" + firmwareVersion + "/system/image/Manual.jpg";
  drawSdJpeg(path.c_str(), 96, 70);
  //drawSdJpeg("/" + firmwareVersion +"/system/image/Manual.jpg", 96, 70);

}
