#include <TFT_eSPI.h> // Include the graphics library (this includes the sprite functions)
#include <Arduino.h>
#include "GlobalModule.h"
#include "GlobalParameter.h"
#include "GlobalFunction.h"

// Adjusted pastel colors for better harmony
#define PASTEL_BG       tft.color565(240, 240, 240) // Light pastel background
#define PASTEL_TEXT     tft.color565(60, 60, 60)    // Soft gray text

void showPage20() {
  // Clear the screen and set background to pastel color
  tft.fillScreen(PASTEL_BG);

  // Display the Settings title with adjusted font and colors
  int midX = tft.width() / 2;
  tft.setTextSize(2);
  tft.setTextColor(PASTEL_TEXT, PASTEL_BG);
  tft.setCursor(midX - 50, 20);
  tft.print("NEWS");

  // Draw Left Arrow
  drawArrow(30, 10, "left");

  // Add text at the bottom, wrapped to fit within the screen
  tft.setTextSize(1);
  tft.setTextDatum(TL_DATUM); // Top Left Datum for text wrapping
  tft.setCursor(120, 200); // Set cursor to start drawing text
  tft.print("Read More");

  drawQRCode("hello",4 ,4 , -20 );
  
}
