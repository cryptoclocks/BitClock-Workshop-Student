#include <TFT_eSPI.h> // Include the graphics library (this includes the sprite functions)
#include <Arduino.h>
#include "GlobalModule.h"
#include "GlobalParameter.h"
#include "GlobalFunction.h"

// Adjusted pastel colors for better harmony
#define PASTEL_BG       tft.color565(240, 240, 240) // Light pastel background
#define PASTEL_TEXT     tft.color565(60, 60, 60)    // Soft gray text
#define PASTEL_SUBTEXT  tft.color565(100, 100, 100) // Darker gray for subtext

int myplan = 0; // 0 = free , 1 = money , 2 = gold , 3 = bitcoin

void showPage11() {
  // Clear the screen and set background to pastel color
  tft.fillScreen(PASTEL_BG);

  // Display the "Your Plans" title with adjusted font and colors
  int midX = tft.width() / 2;
  tft.setTextSize(2);
  tft.setTextColor(PASTEL_TEXT, PASTEL_BG);
  tft.setCursor(midX - 50, 20);
  tft.print("Your Plans");

  // Draw Left Arrow
  drawArrow(30, 10, "left");

  // Define button positions and dimensions with 2 pixel spacing
  int buttonWidth = 140;
  int buttonHeight = 90;
  int buttonX1 = 18; // X position with 2 pixels spacing (left column)
  int buttonX2 = tft.width() - 18 - buttonWidth; // X position for the right column
  int buttonY1 = 48; // Y position for the top row
  int buttonY2 = 140; // Y position for the bottom row

  int cornerRadius = 10;

  // Button labels and subtext
  const char* labels[] = {"FREE", "MONEY", "GOLD", "BITCOIN"};
  const char* subtexts[] = {"", "99 CCP/MONTH", "169 CCP/MONTH", "349 CCP/MONTH"};

  // Define gradient colors for buttons (startColor, endColor)
  uint16_t gradientColors[][2] = {
    {tft.color565(220, 220, 220), tft.color565(200, 200, 200)}, // Pastel Gray for FREE
    {tft.color565(173, 216, 230), tft.color565(135, 206, 250)}, // Pastel Blue for MONEY
    {tft.color565(255, 255, 204), tft.color565(255, 255, 153)}, // Pastel Yellow for GOLD
    {tft.color565(255, 223, 186), tft.color565(255, 165, 79)}   // Pastel Orange for BITCOIN
  };

  // Define darker gradient colors for inactive buttons
  uint16_t darkerGradientColors[][2] = {
    {tft.color565(150, 150, 150), tft.color565(120, 120, 120)}, // Darker Gray for FREE
    {tft.color565(100, 149, 237), tft.color565(70, 130, 180)},  // Darker Blue for MONEY
    {tft.color565(204, 204, 153), tft.color565(204, 204, 102)}, // Darker Yellow for GOLD
    {tft.color565(210, 180, 140), tft.color565(205, 133, 63)}   // Darker Orange for BITCOIN
  };

  // Draw the buttons with their respective gradients and labels
  for (int i = 0; i < 4; i++) {
    // Determine the button's position and gradient based on myplan
    int xPos = (i % 2 == 0) ? buttonX1 : buttonX2;
    int yPos = (i < 2) ? buttonY1 : buttonY2;
    uint16_t* gradient = (i == myplan) ? gradientColors[i] : darkerGradientColors[i];
    uint16_t textColor = (i == myplan) ? TFT_WHITE : PASTEL_TEXT;

    drawGradientRoundedRect(xPos, yPos, buttonWidth, buttonHeight, cornerRadius, gradient[0], gradient[1]);
    
    // Draw the pastel red border if this is the selected plan
    if (i == myplan) {
      tft.drawRoundRect(xPos - 1, yPos - 1, buttonWidth + 2, buttonHeight + 2, cornerRadius + 1, tft.color565(255, 182, 193)); // Pastel Red border
    }
    
    tft.setTextDatum(MC_DATUM); // Set datum to Middle Centre
    tft.setTextColor(textColor);
    tft.drawString(labels[i], xPos + buttonWidth / 2, yPos + buttonHeight / 2);
    
    // Draw subtext if not the FREE plan
    if (i > 0) {
      tft.setTextSize(1); // Smaller text for subtext
      tft.setTextColor(PASTEL_SUBTEXT);
      tft.drawString(subtexts[i], xPos + buttonWidth / 2, yPos + buttonHeight / 2 + 15);
      tft.setTextSize(2); // Reset text size
    }
  }
}
