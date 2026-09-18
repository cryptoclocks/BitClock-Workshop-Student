#include <TFT_eSPI.h> // Include the graphics library (this includes the sprite functions)
#include <Arduino.h>
#include "GlobalModule.h"
#include "GlobalParameter.h"
#include "GlobalFunction.h"
#include <HTTPClient.h>
#include <ArduinoJson.h>

// Adjusted pastel colors for better harmony
#define PASTEL_BG tft.color565(240, 240, 240) // Light pastel background
#define PASTEL_TEXT tft.color565(60, 60, 60)  // Soft gray text

// Adjusted pastel colors for better harmony
#define PASTEL_BG tft.color565(240, 240, 240) // Light pastel background
#define PASTEL_TEXT tft.color565(60, 60, 60)  // Soft gray text

// ตัวแปรที่ใช้ร่วม
char buttonSubText[11][4][20];  // ประกาศไว้ก่อน





void updateButtonSubText() {
  float pricesTHB9[]  = {29, 69, 139, 439};
  float pricesTHB10[] = {939, 1639, 2539, 3639};

  if (region == "thailand")
  {
    for (int i = 0; i < 4; i++) {
      float value9  = pricesTHB9[i];
      float value10 = pricesTHB10[i];

      snprintf(buttonSubText[9][i], 20, "%.2f %s", value9, currency2.c_str());
      snprintf(buttonSubText[10][i], 20, "%.2f %s", value10, currency2.c_str());
    }
  }
  else if (region == "global")
  {
    for (int i = 0; i < 4; i++) {
      float value9  = pricesTHB9[i] / exPayRate;
      float value10 = pricesTHB10[i] / exPayRate;

      snprintf(buttonSubText[9][i], 20, "%.2f %s", value9, currency1.c_str());
      snprintf(buttonSubText[10][i], 20, "%.2f %s", value10, currency1.c_str());
    }
  }

}

// Function to show the dynamic page with specific menuId
void showPage9()
{
  bool showRightArrow = false;
  // Clear the screen and set background to pastel color
  tft.fillScreen(PASTEL_BG);
  Serial.print("→ [DEBUG] Before showPage9(), menuId = ");
  Serial.println(menuId);
  // Define titles based on menuId
  const char *titles[] = {"Settings1", "Settings2", "Settings3", "Settings4", "Method", "Profile", "WALLET", "SOCIAL", "CONTACT", "TOPUP", "TOPUP"};
  const char *title = titles[menuId];

  // Define button labels for each menuId
  const char *buttonSets[11][4] = {
    {"NOTI", "NEWS", "HORO", "GAME"},               // menuId = 0
    {"TOKEN", "PLAN", "MAIL", "ACC"},               // menuId = 1
    {"VOTE", "BILL", "OTA", "REFER"},               // menuId = 2
    {"MANUAL", "SHOP", "FAQ", "SYSTEM"},            // menuId = 3
    {"STRIPE", "USDT", "SAT", "CCT"},               // menuId = 4
    {"WALLET", "SOCIAL", "CONTACT", "BADGE"},       // menuId = 5
    {"PROMPTPAY", "USDT", "SAT", "CCT"},            // menuId = 6
    {"FACEBOOK", "YOUTUBE", "TIKTOK", "INSTAGRAM"}, // menuId = 7
    {"EMAIL/TEL", "WECHAT", "LINE", "WHATSAPP"},    // menuId = 8
    {"29 CCP", "69 CCP", "139 CCP", "439 CCP"},     // menuId = 9
    {"939 CCP", "1639 CCP", "2539 CCP", "3639 CCP"} // menuId = 10
  };

  // Define small text for each button (initially blank)

  updateButtonSubText();



  // Define gradient colors for buttons
  uint16_t buttonColors[11][4][2] = {
    // menuId = 0
    {
      {tft.color565(255, 255, 153), tft.color565(255, 204, 102)}, // NOTI (Yellow)
      {tft.color565(153, 204, 255), tft.color565(102, 153, 255)}, // NEWS (Blue)
      {tft.color565(204, 153, 255), tft.color565(153, 102, 255)}, // HORO (Purple)
      {tft.color565(153, 255, 153), tft.color565(102, 204, 102)}  // GAME (Green)
    },
    // menuId = 1
    {
      {tft.color565(255, 204, 153), tft.color565(255, 153, 102)}, // TOKEN (Orange)
      {tft.color565(255, 153, 204), tft.color565(255, 102, 153)}, // PET (Pink)
      {tft.color565(153, 204, 255), tft.color565(102, 153, 255)}, // MAIL (Blue)
      {tft.color565(200, 200, 200), tft.color565(150, 150, 150)}  // ACCOUNT (Gray)
    },
    // menuId = 2
    {
      {tft.color565(153, 255, 153), tft.color565(102, 204, 102)}, // VOTE (Green)
      {tft.color565(255, 153, 153), tft.color565(255, 102, 102)}, // BILLING (Red)
      {tft.color565(204, 153, 255), tft.color565(153, 102, 255)}, // OTA (Purple)
      {tft.color565(153, 204, 255), tft.color565(102, 153, 255)}  // MANUAL (Blue)
    },
    // menuId = 3
    {
      {tft.color565(153, 255, 153), tft.color565(102, 204, 102)}, // REFER (Green)
      {tft.color565(255, 255, 153), tft.color565(255, 204, 102)}, // HELP (Yellow)
      {tft.color565(255, 204, 153), tft.color565(255, 153, 102)}, // FAQ (Orange)
      {tft.color565(200, 200, 200), tft.color565(150, 150, 150)}  // SYSTEM (Gray)
    },
    // menuId = 4
    {
      {tft.color565(255, 153, 153), tft.color565(255, 102, 102)}, // CCP (Red)
      {tft.color565(153, 255, 153), tft.color565(102, 204, 102)}, // CCT (Green)
      {tft.color565(153, 204, 255), tft.color565(102, 153, 255)}, // SOL (Blue)
      {tft.color565(204, 153, 255), tft.color565(153, 102, 255)}  // SAT (Purple)
    },
    // menuId = 5
    {
      {tft.color565(255, 255, 204), tft.color565(255, 204, 153)}, // WALLET (Light Yellow)
      {tft.color565(204, 204, 255), tft.color565(153, 153, 255)}, // SOCIAL (Light Purple)
      {tft.color565(255, 204, 204), tft.color565(255, 153, 153)}, // CONTACT (Light Red)
      {tft.color565(153, 255, 204), tft.color565(102, 204, 153)}  // BADGE (Light Green)
    },
    // menuId = 6
    {
      {tft.color565(153, 204, 255), tft.color565(102, 153, 255)}, // PROMPTPAY (Green)
      {tft.color565(102, 204, 153), tft.color565(51, 153, 102)},  // USDT (Green)
      {tft.color565(204, 153, 255), tft.color565(153, 102, 255)}, // SAT (Purple)
      {tft.color565(204, 255, 204), tft.color565(153, 255, 153)}  // CCT (Blue)
    },
    // menuId = 7
    {
      {tft.color565(153, 204, 255), tft.color565(102, 153, 255)}, // FACEBOOK (Blue)
      {tft.color565(255, 204, 153), tft.color565(255, 153, 102)}, // YOUTUBE (Orange)
      {tft.color565(255, 153, 204), tft.color565(255, 102, 153)}, // TIKTOK (Pink)
      {tft.color565(204, 255, 204), tft.color565(153, 255, 153)}  // INSTAGRAM (Green)
    },
    // menuId = 8
    {
      {tft.color565(204, 255, 255), tft.color565(153, 204, 255)}, // EMAIL/TEL (Light Blue)
      {tft.color565(255, 204, 204), tft.color565(255, 153, 153)}, // WECHAT (Light Red)
      {tft.color565(204, 255, 204), tft.color565(153, 255, 153)}, // LINE (Light Green)
      {tft.color565(255, 255, 153), tft.color565(255, 204, 102)}  // WHATSAPP (Yellow)
    },
    // menuId = 9
    {
      {tft.color565(204, 255, 255), tft.color565(153, 204, 255)}, // 29 (Light Blue)
      {tft.color565(255, 204, 204), tft.color565(255, 153, 153)}, // 69 (Light Red)
      {tft.color565(204, 255, 204), tft.color565(153, 255, 153)}, // 139 (Light Green)
      {tft.color565(255, 255, 153), tft.color565(255, 204, 102)}  // 439 (Yellow)
    },
    // menuId =10
    {
      {tft.color565(204, 255, 255), tft.color565(153, 204, 255)}, // 939 (Light Blue)
      {tft.color565(255, 204, 204), tft.color565(255, 153, 153)}, // 1639 (Light Red)
      {tft.color565(204, 255, 204), tft.color565(153, 255, 153)}, // 2539 (Light Green)
      {tft.color565(255, 255, 153), tft.color565(255, 204, 102)}  // 3639 (Yellow)
    }
  };



  Serial.print("menuId:");
  Serial.println(menuId);
  // แสดงผลใน buttonSubText
  if (menuId == 4) //method
  {
    int basePrice = selectedPrice;
    Serial.print("basePrice:");
    Serial.println(basePrice);
    Serial.print("region:");
    Serial.println(region);
    Serial.print("currencyIndex:");
    Serial.println(currencyIndex);
    if (region == "thailand") { //thai bath
      Serial.println("thai promptpay:");
      buttonSets[4][0] = "PROMPTPAY";
      sprintf(buttonSubText[4][0], "%.2f THB", stripePrice);           // STRIPE
      sprintf(buttonSubText[4][1], "%.2f USDT", usdtPrice);         // USDT
      sprintf(buttonSubText[4][2], "%d SAT", satoshiPrice);         // SAT
      sprintf(buttonSubText[4][3], "%.2f CCT", cctPrice );
    }
    else if (region == "global") //usd
    {
      Serial.println("global STRIPE:");
      buttonSets[4][0] = "STRIPE";
      sprintf(buttonSubText[4][0], "%.2f USD", stripePrice);
      sprintf(buttonSubText[4][1], "%.2f USDT", usdtPrice);  // USDT
      sprintf(buttonSubText[4][2], "%d SAT", satoshiPrice);  // SAT
      sprintf(buttonSubText[4][3], "%.0f CCT", cctPrice);    // CCT
    }
  }
  // Display the dynamic title
  int midX = tft.width() / 2;
  tft.setTextSize(2);
  tft.setTextColor(PASTEL_TEXT, PASTEL_BG);
  tft.setTextDatum(MC_DATUM); // Middle Centre Datum

  // คำนวณตำแหน่งกึ่งกลางของจอ
  int centerX = tft.width() / 2; // กึ่งกลางหน้าจอในแนวนอน
  int centerY = 20;              // ความสูงที่ต้องการสำหรับ title

  // วาดข้อความ
  tft.drawString(title, centerX, centerY);

  // Draw Left Arrow (always shown)
  drawArrow(30, 10, "left");

  // Draw Right Arrow if needed
  if (menuId == 0 || menuId == 1 || menuId == 2 || menuId == 9)
  {
    drawArrow(tft.width() - 30, 10, "right");
  }

  // Define button positions and dimensions
  int buttonWidth = 140;
  int buttonHeight = 90;
  int buttonX1 = 18; // X position
  int buttonX2 = tft.width() - 18 - buttonWidth;
  int buttonY1 = 48; // Y position
  int cornerRadius = 10;
  // Loop to draw each button
  for (int i = 0; i < 2; i++)
  {
    // Draw first column of buttons
    drawGradientRoundedRect(buttonX1, buttonY1 + i * (buttonHeight + 2), buttonWidth, buttonHeight, cornerRadius, buttonColors[menuId][i][0], buttonColors[menuId][i][1]);
    tft.setTextSize(2);         // Small text
    tft.setTextDatum(MC_DATUM); // Set datum to Middle Centre
    tft.setTextColor(PASTEL_TEXT);
    tft.drawString(buttonSets[menuId][i], buttonX1 + buttonWidth / 2, buttonY1 + i * (buttonHeight + 2) + buttonHeight / 2);

    // Draw small text below button
    tft.setTextSize(1);         // Small text
    tft.setTextDatum(MC_DATUM); // Set datum to Middle Centre
    tft.drawString(buttonSubText[menuId][i], buttonX1 + buttonWidth / 2, buttonY1 + i * (buttonHeight + 2) + buttonHeight / 2 + 15);

    // Draw second column of buttons
    drawGradientRoundedRect(buttonX2, buttonY1 + i * (buttonHeight + 2), buttonWidth, buttonHeight, cornerRadius, buttonColors[menuId][i + 2][0], buttonColors[menuId][i + 2][1]);
    tft.setTextSize(2);         // Small text
    tft.setTextDatum(MC_DATUM); // Set datum to Middle Centre
    tft.setTextColor(PASTEL_TEXT);
    tft.drawString(buttonSets[menuId][i + 2], buttonX2 + buttonWidth / 2, buttonY1 + i * (buttonHeight + 2) + buttonHeight / 2);

    // Draw small text below second column button
    tft.setTextSize(1);         // Small text
    tft.setTextDatum(MC_DATUM); // Set datum to Middle Centre
    tft.drawString(buttonSubText[menuId][i + 2], buttonX2 + buttonWidth / 2, buttonY1 + i * (buttonHeight + 2) + buttonHeight / 2 + 15);
  }
}
