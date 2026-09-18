#include <TFT_eSPI.h> // Include the graphics library (this includes the sprite functions)
#include <Arduino.h>
#include "GlobalModule.h"
#include "GlobalParameter.h"
#include "GlobalFunction.h"
#include "page2.h"
#include "cc_littlefs.h"

// Adjusted pastel colors for better harmony
#define PASTEL_BG       tft.color565(240, 240, 240) // Light pastel background
#define PASTEL_TEXT     tft.color565(60, 60, 60)    // Soft gray text
#define PASTEL_SUBTEXT  tft.color565(100, 100, 100) // Darker gray for subtext
#define PASTEL_RED      tft.color565(255, 182, 193) // Light pastel red for the border

// Define button positions and dimensions
int buttonWidth = 108;
int buttonHeight = 54;
int buttonX1 = 14; // X position (left column)
int buttonX2 = 200; // X position (right column)
int buttonY1 = 8;   // Y position (top row)
int buttonY2 = 100; // Y position (second row)
int cornerRadius = 5;

// Function to format numbers according to the specified conditions
String formatNumber(float number) {
  String formattedNum;

  if (number >= 1000000) {
    formattedNum = String(number / 1000000.0, 2) + "M";
  } else if (number >= 10000) {
    formattedNum = String((int)number);
  } else if (number >= 1000) {
    formattedNum = String(number, 1);
  } else if (number >= 100) {
    formattedNum = String(number, 2);
  } else if (number < 100 && number >= 0.001) {
    formattedNum = String(number, 4);
  } else if (number >= 0.000001) {
    formattedNum = String(number * 1000000.0, 0) + "u";
  } else if (number >= 0.000000001) {
    formattedNum = String(number * 1000000000.0, 0) + "n";
  } else {
    formattedNum = "0";
  }

  if (formattedNum.indexOf('.') != -1) {
    while (formattedNum.endsWith("0")) {
      formattedNum.remove(formattedNum.length() - 1);
    }
    if (formattedNum.endsWith(".")) {
      formattedNum.remove(formattedNum.length() - 1);
    }
  }

  if (formattedNum.length() > 11) {
    formattedNum = formattedNum.substring(0, 11);
  }

  return formattedNum;
}





// Function to save input text to alert
void saveInputTextToAlert(int index, String newText) {
  float newValue = newText.toFloat();
  int startIndex = (currentAlertPage - 1) * 8; // คำนวณค่าเริ่มต้นของหน้า
Serial.println("before");

  if ((startIndex + index) % 2 == 0) {
    coinLow[coinIndex][(startIndex + index) / 2] = newValue;
  } else {
    coinHigh[coinIndex][(startIndex + index) / 2] = newValue;
  }

}

// Function to draw selected text with status
void drawSelectedText(int x, int y, const char* text, bool isSelected, int index) {
  int textWidth = 83;
  int textHeight = 16;
  int padding = 5;

  tft.setTextDatum(MC_DATUM);
  bool status = (index % 2 == 0)
                ? coinLowStatus[coinIndex][index / 2]
                : coinHighStatus[coinIndex][index / 2];

  uint16_t statusColor = status ? tft.color565(144, 238, 144) : tft.color565(255, 182, 193); // Green if ON, Red if OFF

  if (isSelected) {
    tft.fillRect(x - 50 - textWidth / 2 - padding + 1, y - textHeight / 2 - padding / 2 - 2 + 10 - 2, textWidth + 2 * padding - 2, 3, TFT_CYAN);
    tft.fillRect(x - 50 - textWidth / 2 - padding + 1, y - textHeight / 2 - padding / 2 - 2 + 10 + 1, textWidth + 2 * padding - 2, textHeight + padding + 3 - 2, statusColor);
    tft.setTextColor(TFT_WHITE, statusColor);
  } else {
    tft.fillRect(x - 50 - textWidth / 2 - padding + 1, y - textHeight / 2 - padding / 2 - 2 + 10 - 2, textWidth + 2 * padding - 2, 3, TFT_BLACK);
    tft.fillRect(x - 50 - textWidth / 2 - padding + 1, y - textHeight / 2 - padding / 2 - 2 + 10 + 1, textWidth + 2 * padding - 2, textHeight + padding + 3 - 2, statusColor);
    tft.setTextColor(TFT_WHITE, statusColor);
  }

  tft.drawString(text, x - 50, y + 10);
}

void updateSelectedText(int index) {
  Serial.print("coinIndex:");
  Serial.println(coinIndex);

  int startIndex = (currentAlertPage - 1) * 8;

  Serial.println("");
  for (int i = 0; i < 8; i++) {
    int x = 160 + (i % 2) * 94;
    int y = 120 + (i / 2) * 30;

    // การคำนวณ index สำหรับ array
    int actualIndex = startIndex + i;

    float valueToShow = actualIndex % 2 == 0 ? coinLow[coinIndex][actualIndex / 2] : coinHigh[coinIndex][actualIndex / 2];
    bool statusToShow = actualIndex % 2 == 0 ? coinLowStatus[coinIndex][actualIndex / 2] : coinHighStatus[coinIndex][actualIndex / 2];

    String valueString = formatNumber(valueToShow);
    drawSelectedText(x, y, valueString.c_str(), i == alertSelectIndex, actualIndex);

    if (i == alertSelectIndex) {
      uint16_t bgcolor1 = statusToShow ? tft.color565(255, 182, 193) : tft.color565(144, 238, 144);
      drawCircleWithText(30, 205, 30, bgcolor1, TFT_WHITE, TFT_WHITE, statusToShow ? "OFF" : "ON");
    }
  }
}



// Function to toggle the selected coin
void toggleCoin() {
  coinIndex = (coinIndex == maxCoin-1) ? 0 : coinIndex + 1;

  int xPos = buttonX1;
  int yPos = buttonY1;
  uint16_t textColor = TFT_WHITE;

  drawGradientRoundedRect(xPos, yPos, buttonWidth, buttonHeight, cornerRadius, tft.color565(255, 223, 186), tft.color565(255, 165, 79));
  drawGradientRoundedRect(xPos + 5, yPos + 5, buttonWidth - 10, buttonHeight - 10, cornerRadius - 2, TFT_BLACK, TFT_BLACK);

  tft.setTextDatum(MC_DATUM);
  tft.setTextColor(textColor);
  coinType = type[coinIndex].toInt() ;
  tft.drawString(coin[coinIndex], xPos + buttonWidth / 2, yPos + buttonHeight / 2);
  showLogoAtTopMiddle();
  showLogoAtBottomMiddle();
  updateSelectedText(alertSelectIndex);
}

// ฟังก์ชันสำหรับจัดการกับทศนิยมตามเงื่อนไขที่กำหนด
float formatDecimal(float value) {
  if (value >= 100000.0) {
    return round(value); // ตัดทศนิยมทิ้ง
  } else if (value >= 10000.0) {
    return round(value * 10) / 10.0; // เหลือทศนิยม 1 ตำแหน่ง
  } else if (value >= 1000.0) {
    return round(value * 100) / 100.0; // เหลือทศนิยม 2 ตำแหน่ง
  } else if (value >= 100.0) {
    return round(value * 1000) / 1000.0; // เหลือทศนิยม 3 ตำแหน่ง
  } else if (value >= 10.0) {
    return round(value * 10000) / 10000.0; // เหลือทศนิยม 4 ตำแหน่ง
  } else {
    return round(value * 10000) / 10000.0; // เหลือทศนิยมสูงสุด 4 ตำแหน่ง
  }
}

//void toggleCurrency() {
//  // สลับ currency สำหรับ alert
//  if (currencyAlert == currency2) {
//    currencyAlert = currency1;
//  } else {
//    currencyAlert = currency2;
//  }
//
//  // วาดปุ่มใหม่
//  drawGradientRoundedRect(buttonX2, buttonY1, buttonWidth, buttonHeight, cornerRadius,
//                          tft.color565(255, 223, 186), tft.color565(255, 165, 79));
//  drawGradientRoundedRect(buttonX2 + 5, buttonY1 + 5, buttonWidth - 10, buttonHeight - 10,
//                          cornerRadius - 2, TFT_BLACK, TFT_BLACK);
//
//  tft.setTextDatum(MC_DATUM);
//  tft.setTextColor(TFT_WHITE);
//  tft.drawString(currencyAlert, buttonX2 + buttonWidth / 2, buttonY1 + buttonHeight / 2);
//
//  // ใช้ index ของสกุลเงินในการเลือก exRate ที่ถูกต้อง
//  int currentCurrencyIndex = (currencyAlert == currency2) ? currencyIndex : (currencyIndex == 0 ? 1 : 0);
//
//  for (int i = 0; i < 4; i++) {
//    float rate = exRate[coinIndex][currentCurrencyIndex];
//    coinLow[coinIndex][i]  = formatDecimal(coinLow[coinIndex][i]  * rate);
//    coinHigh[coinIndex][i] = formatDecimal(coinHigh[coinIndex][i] * rate);
//  }
//
//  updateSelectedText(alertSelectIndex);
//  save_alert();
//}


// Function to toggle the ON/OFF status
void toggleStatus(int index) {
  int startIndex = (currentAlertPage - 1) * 8; // คำนวณค่าเริ่มต้นของหน้า

  if ((startIndex + index) % 2 == 0) {
    coinLowStatus[coinIndex][(startIndex + index) / 2] = !coinLowStatus[coinIndex][(startIndex + index) / 2];
  } else {
    coinHighStatus[coinIndex][(startIndex + index) / 2] = !coinHighStatus[coinIndex][(startIndex + index) / 2];
  }

  updateSelectedText(index);
  save_alert();
}


// Function to display page 12
void showPage12() {
  tft.fillScreen(TFT_BLACK);
  tft.setTextSize(2);
  tft.setTextColor(PASTEL_TEXT, PASTEL_BG);

  // Draw Left Arrow
  drawArrow(25, 68, "left");

  // Draw the buttons with their respective gradients and labels
  for (int i = 0; i < 2; i++) {
    int xPos = (i % 2 == 0) ? buttonX1 : buttonX2;
    int yPos = (i < 2) ? buttonY1 : buttonY2;
    uint16_t textColor = TFT_WHITE;
    drawGradientRoundedRect(xPos, yPos, buttonWidth, buttonHeight, cornerRadius, tft.color565(255, 223, 186), tft.color565(255, 165, 79));
    drawGradientRoundedRect(xPos + 5, yPos + 5, buttonWidth - 10, buttonHeight - 10, cornerRadius - 2, TFT_BLACK, TFT_BLACK);
    tft.setTextDatum(MC_DATUM);
    tft.setTextColor(textColor);

    if (i == 0) {

      tft.drawString(coin[coinIndex], xPos + buttonWidth / 2, yPos + buttonHeight / 2);
    } else {
      tft.drawString(currencyAlert, xPos + buttonWidth / 2, yPos + buttonHeight / 2);
    }
  }

  showLogoAtTopMiddle();

  // Draw blue up arrow
  drawCircleWithText(285, 140, 30, tft.color565(173, 255, 47), TFT_WHITE, TFT_WHITE, "UP");

  // Draw blue down arrow
  drawCircleWithText(285, 205, 30, tft.color565(255, 160, 122), TFT_WHITE, TFT_WHITE, "DWN");

  // Draw circle with "SET"
  drawCircleWithText(30, 140, 30, tft.color565(173, 216, 230), TFT_WHITE, TFT_WHITE, "SET");

  bool statusToShow = (alertSelectIndex % 2 == 0)
                      ? coinLowStatus[coinIndex][alertSelectIndex / 2]
                      : coinHighStatus[coinIndex][alertSelectIndex / 2];

  uint16_t onColor = statusToShow ? tft.color565(144, 238, 144) : tft.color565(255, 182, 193); // Green if ON, Red if OFF
  drawCircleWithText(30, 205, 30, onColor, TFT_WHITE, TFT_WHITE, statusToShow ? "ON" : "OFF");

  tft.setTextSize(1);
  tft.setTextColor(TFT_WHITE);
  tft.drawString("Notification", 162, 83);

  tft.setTextColor(TFT_GREEN);
  tft.drawString("LOW", 114, 98);
  tft.setTextColor(TFT_RED);
  tft.drawString("HIGH", 209, 98);
  tft.setTextSize(2);

  updateSelectedText(alertSelectIndex);
}
