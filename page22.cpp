#include <TFT_eSPI.h> // Include the graphics library (this includes the sprite functions)
#include <Arduino.h>
#include "GlobalModule.h"
#include "GlobalParameter.h"
#include "GlobalFunction.h"
#include "JpgSupport.h"
#include "JPEGDecoder.h"

// Adjusted pastel colors for better harmony
#define PASTEL_BG       tft.color565(240, 240, 240) // Light pastel background




void showPage22() {
  // Clear the screen and set background to black
  tft.fillScreen(TFT_BLACK);

  // Display the "ALERT" title
  tft.setTextSize(4);
  if (alertStatus == "LOW")
  {
    tft.setTextColor(TFT_GREEN, TFT_BLACK);
    tft.setTextDatum(ML_DATUM);
    tft.drawString("!ALERT!", tft.width() / 2 - 74, 30);
    tft.setTextSize(2);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.drawString("Price: " + formatAlertPrice(coinLow[alertI][alertJ]) + " " + currencyAlert, 20, 110);
  }
  else
  {
    tft.setTextColor(TFT_RED, TFT_BLACK);
    tft.setTextDatum(ML_DATUM);
    tft.drawString("!ALERT!", tft.width() / 2 - 74, 30);
    tft.setTextSize(2);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.drawString("Price: " + formatAlertPrice(coinHigh[alertI][alertJ]) + " " + currencyAlert, 20, 110);
  }


  // Display the alert message using global parameters


  tft.setTextDatum(ML_DATUM);
  tft.drawString("Coin: " + alertCoin, 20, 70); // Using the global `coin` array and `coinIndex`
  tft.drawString("Status: " + alertStatus, 160, 70);

  tft.drawString("Alert Price: " + alertPrice + " " + currencyAlert, 20, 150);  // Adjust the position of the alert message based on its length

  //tft.drawString("Total CCP: " + String(ccp_balance), 20, 150);

  // Draw the "DISMISS" button
  drawGradientRoundedRect(10, 175, 110, 60, 10, tft.color565(255, 182, 193), tft.color565(255, 105, 180));
  tft.setTextColor(TFT_WHITE);
  tft.drawString("DISMISS", 26, 205);

  // Draw the "SNOOZE" button
  drawGradientRoundedRect(200, 175, 110, 60, 10, tft.color565(144, 238, 144), tft.color565(60, 179, 113));
  tft.setTextColor(TFT_WHITE);
  tft.drawString("SNOOZE", 220, 205);
}
