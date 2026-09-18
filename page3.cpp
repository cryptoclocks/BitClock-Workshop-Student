
#include "GlobalModule.h"
#include "GlobalParameter.h"
#include "GlobalFunction.h"
#include "page2.h"
// Global brightness level (0-100)
// Interval options in seconds




void displaySystemInfo() {
  char formattedNumber[7]; // สร้างตัวแปรสตริงที่มีความยาวเพียงพอ (6 ตัวอักษร + 1 สำหรับ null terminator)

  tft.setTextSize(2);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setCursor(10, 20);
  tft.print("ID: ");
  tft.println(deviceId); // Assume `deviceId` is a String variable

  // Display Firmware Version
  tft.setCursor(200, 20);
  tft.print("FW: v");
  tft.println(firmwareVersion); // Assume `firmwareVersion` is a String variable

  // Display IP Address
  tft.setCursor(10, 60);
  tft.print("IP: ");
  tft.println(ipAddress); // Assume `ipAddress` is a String variable



}

void displayBrightness() {
  char brightnessText[30];
  sprintf(brightnessText, "Brightness: %d%%", brightnessLevel);
  // Display brightness slider
  tft.setTextSize(2);
  tft.setCursor(10, 100);
  tft.print(brightnessText);
  int textYCenter = 100 + 8;

  tft.fillTriangle(
    285, textYCenter + 5, // Top point (left of the text)
    280, textYCenter - 5, // Bottom right point
    290, textYCenter - 5, // Bottom left point
    TFT_RED);

  // Assuming the text width is variable, we dynamically place the 'up' button.
  // For simplicity, let's place it 200 pixels from the left edge, adjust as needed.
  tft.fillTriangle(
    245, textYCenter - 5, // Top point (right side, adjust as necessary)
    240, textYCenter + 5, // Bottom right point
    250, textYCenter + 5, // Bottom left point
    TFT_GREEN);
}


void displayIntervalSelection() {
  char intervalText[40];
  sprintf(intervalText, "Interval: %ld", updateInterval / 1000);
  tft.setTextSize(2);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setCursor(10, 140); // Starting text position
  tft.print(intervalText);

  if (intervals[currentIntervalIndex] / 1000 < 60) {
    //tft.print(intervals[currentIntervalIndex]);
    tft.print(" sec");
  } else {
    //tft.print(intervals[currentIntervalIndex] / 60);
    tft.print(" min");
  }

  // Assuming the text height is around 16 pixels,
  // we adjust the triangle to be centered around the text's Y position + half its height.
  // Adjust the 130 value if your text's baseline is different.
  int textYCenter = 140 + 8; // Roughly center of the text's height

  // Display 'down' button aligned with text
  // Adjusting triangle's position to be centered with text's height
  tft.fillTriangle(
    285, textYCenter + 5, // Top point (left of the text)
    280, textYCenter - 5, // Bottom right point
    290, textYCenter - 5, // Bottom left point
    TFT_RED);

  // Assuming the text width is variable, we dynamically place the 'up' button.
  // For simplicity, let's place it 200 pixels from the left edge, adjust as needed.
  tft.fillTriangle(
    245, textYCenter - 5, // Top point (right side, adjust as necessary)
    240, textYCenter + 5, // Bottom right point
    250, textYCenter + 5, // Bottom left point
    TFT_GREEN);
}

void displayTimeout() {
  char timeoutText[40];
  sprintf(timeoutText, "Timeout: %ld", timeout / 1000);


  tft.setTextSize(2);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setCursor(10, 180); // Starting text position
  tft.print(timeoutText);

  if (timeouts[currentTimeoutIndex] / 1000 < 60) {
    //tft.print(timeouts[currentTimeoutIndex]);
    tft.print(" sec");
  } else {
    //tft.print(timeouts[currentTimeoutIndex] / 60000);
    tft.print(" min");
  }

  // Assuming the text height is around 16 pixels,
  // we adjust the triangle to be centered around the text's Y position + half its height.
  // Adjust the 130 value if your text's baseline is different.
  int textYCenter = 180 + 8; // Roughly center of the text's height

  // Display 'down' button aligned with text
  // Adjusting triangle's position to be centered with text's height
  tft.fillTriangle(
    285, textYCenter + 5, // Top point (left of the text)
    280, textYCenter - 5, // Bottom right point
    290, textYCenter - 5, // Bottom left point
    TFT_RED);

  // Assuming the text width is variable, we dynamically place the 'up' button.
  // For simplicity, let's place it 200 pixels from the left edge, adjust as needed.
  tft.fillTriangle(
    245, textYCenter - 5, // Top point (right side, adjust as necessary)
    240, textYCenter + 5, // Bottom right point
    250, textYCenter + 5, // Bottom left point
    TFT_GREEN);
}

void displayCredit() {
  tft.setTextSize(1);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setCursor(185, 230); // Starting text position
  tft.print("by CryptoClockTeam");
  tft.setTextSize(2);


}

void drawBlueArrow() {
  int startX = 50; // Starting X position of the arrow on the display
  int startY = tft.height() - 35; // Starting Y position (30 pixels from the bottom)
  int arrowWidth = 20; // Width of the arrow
  int arrowHeight = 30; // Height of the arrow

  // Arrow body (rectangle)
  tft.fillRect(startX, startY + 10, arrowWidth, arrowHeight - 20, TFT_BLUE);

  // Arrow head (triangle)
  tft.fillTriangle(
    startX - 10, startY + (arrowHeight / 2), // Left point (tip of the arrow)
    startX, startY + 5, // Right top point
    startX, startY + arrowHeight - 5, // Right bottom point
    TFT_BLUE);
}

void showPage3() {
  tft.fillScreen(TFT_BLACK);
  displaySystemInfo();
  displayBrightness();
  displayIntervalSelection();
  displayTimeout();
  displayCredit();
  drawBlueArrow();
}
