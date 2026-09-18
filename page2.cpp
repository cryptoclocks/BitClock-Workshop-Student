#include <Arduino.h>
#include "GlobalModule.h"
#include "GlobalParameter.h"
#include "GlobalFunction.h"
#include "time.h"
#include "page1.h"
#include "page6.h"
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <FS.h>
#include "JpgSupport.h"
#define MAX_IMAGE_WIDTH 320

// BEEP TEST:
// Set either skip flag to 1 temporarily to confirm whether SD/JPG drawing
// is the source of the small passive-buzzer chirp when entering/redrawing page 2.
#define PAGE2_BEEP_TEST_CLAMP_SD_DRAW 1
#define PAGE2_BEEP_TEST_SKIP_BACKGROUND_SD 1
#define PAGE2_BEEP_TEST_SKIP_LOGO_SD 0

static void clampBuzzerForPage2Draw() {
#if PAGE2_BEEP_TEST_CLAMP_SD_DRAW
  silenceBuzzer();
#endif
}

int mainPriceDigits = 0;

int countDigitsInString(String s) {
  int c = 0;
  for (int i = 0; i < s.length(); i++) {
    if (s[i] >= '0' && s[i] <= '9') {
      c++;
    }
  }
  return c;
}

int countIntegerDigits(float value) {
  unsigned long temp = (unsigned long)fabs(value);
  int digits = 1;
  while (temp >= 10) {
    temp /= 10;
    digits++;
  }
  return digits;
}

void updateTime2() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Failed to obtain time");
    return;
  }

  char timeString[20];
  strftime(timeString, sizeof(timeString), "%H:%M:%S %d/%m/%Y", &timeinfo);
  Serial.println(timeString); // Print the time to the Serial Monitor

  // Display the time on the TFT
  tft.setCursor(100, 85);
  tft.setTextColor(TFT_WHITE); // Set text color
  tft.setTextSize(1); // Set text size
  tft.print(timeString);

  if (lotteryAlert == true) {
    Serial.println("LotteryOn");
    // Check for lottery days and 5 PM
    int day1 = timeinfo.tm_mday;
    int hour1 = timeinfo.tm_hour;
    int minute1 = timeinfo.tm_min;
    //    Serial.print("day:");
    //    Serial.println(day1);
    //    Serial.print("hour:");
    //    Serial.println(hour1);
    //    Serial.print("minute:");
    //    Serial.println(minute1);
    if ((day1 == 1 || (day1 == 2 && duplottery == false) || day1 == 16 || (day1 == 17 && duplottery == false)) && hour1 == 17 && minute1 == 31 && pageIndex != 5 ) {
      if (day1 == 1 || day1 == 16)
      {
        duplottery = true;
      }
      //stop task
      stopPriceTask();
      pageIndex = 6;
      delay(1000);
      pageIndex = 6;
      Serial.println("Lottery time!");
      showPage6();
    } else if ((day1 == 3 || day1 == 18) && duplottery == true) {
      //clear stack
      duplottery = false;
    }
  } else {
    Serial.println("LotteryOff");
  }

  tft.setTextSize(2);
  Serial.println("updated time finish");
}

void displayBackgroundImageForPage2() {
  String imagePath = "/" + firmwareVersion + "/system/image/page2.jpg"; // Default background image
  if (coin[coinIndex] == "GOLD") {
    imagePath = "/" + firmwareVersion + "/system/image/goldbg.jpg";
    clampBuzzerForPage2Draw();
#if PAGE2_BEEP_TEST_SKIP_BACKGROUND_SD
    Serial.println("[BEEP TEST] Skip page2 background JPG draw");
    tft.fillScreen(TFT_BLACK);
#else
    drawSdJpeg(imagePath.c_str(), 0, 0);
#endif
    clampBuzzerForPage2Draw();
  }
  else if (coin[coinIndex] == "ZENT")
  {
    imagePath = "/" + firmwareVersion + "/system/image/pagecm.jpg";
  clampBuzzerForPage2Draw();

    drawSdJpeg(imagePath.c_str(), 0, 0);

    clampBuzzerForPage2Draw();

    


  }
  else
  {
    clampBuzzerForPage2Draw();
#if PAGE2_BEEP_TEST_SKIP_BACKGROUND_SD
    Serial.println("[BEEP TEST] Skip page2 background JPG draw");
    tft.fillScreen(TFT_BLACK);
#else
    drawSdJpeg(imagePath.c_str(), 0, 0);
#endif
    clampBuzzerForPage2Draw();
  }

}

void drawRoundedBoxOnly(int x, int y, int boxW, int boxH, int radius, int borderThickness, uint16_t borderColor) {
  // วาดขอบโค้งหนาหลายชั้น
  for (int i = 0; i < borderThickness; i++) {
    tft.drawRoundRect(x - i, y - i, boxW + i * 2, boxH + i * 2, radius + i, borderColor);
  }

  // วาดพื้นหลังดำตรงกลาง
  tft.fillRoundRect(x + borderThickness, y + borderThickness,
                    boxW - 2 * borderThickness, boxH - 2 * borderThickness,
                    radius, TFT_BLACK);
}


void displayCoinOnScreen() {

  int boxW = 95;  // สำหรับข้อความไม่เกิน 6 ตัวอักษร
  int boxH = 48;
  int radius = 6;
  int borderThickness = 5;
  uint16_t borderColor = convertColor(color[coinIndex]);

  if (coin[coinIndex] != "ZENT" && coin[coinIndex] != "GOLD")
  {
    // ===== Coin =====
    int coinX = 35;
    int coinY = 18;
    drawRoundedBoxOnly(coinX - 15, coinY - 4, boxW, boxH, radius, borderThickness, borderColor);


    // ===== Currency =====
    int currencyX = 226;
    int currencyY = 18;
    drawRoundedBoxOnly(currencyX - 20, currencyY - 4, boxW, boxH, radius, borderThickness, borderColor);

  }


  tft.setTextSize(2);
  int xPosition = 59; // Default X position for the coin name

  // Adjust the cursor position based on the length of the coin name
  if (coin[coinIndex].length() == 1) {
    xPosition = 59;
  } else if (coin[coinIndex].length() == 2) {
    xPosition = 55;
  } else if (coin[coinIndex].length() == 3) {
    xPosition = 51;
  } else if (coin[coinIndex].length() == 4) {
    xPosition = 47;
  } else if (coin[coinIndex].length() == 5) {
    xPosition = 43;
  } else if (coin[coinIndex].length() == 6) {
    xPosition = 35;
  } else if (coin[coinIndex].length() == 7) {
    xPosition = 27;
  }
  tft.setCursor(xPosition, 30);
  if (coin[coinIndex] == "USD")
  {
    tft.print("USDT");
  }

  else
  {
    tft.print(coin[coinIndex]);
  }


  xPosition = 242; // Default X position for the currency name

  // Adjust the cursor position based on the length of the currency name
  if (coin[coinIndex] == "GOLD")
  {
    currencyIndex = 1 ;
    currency[currencyIndex] = "THB";
  }
  if (currency[currencyIndex].length() == 2) {
    xPosition = 242;
  } else if (currency[currencyIndex].length() == 3) {
    xPosition = 238;
  } else if (currency[currencyIndex].length() == 4) {
    xPosition = 234;
  } else if (currency[currencyIndex].length() == 5) {
    xPosition = 230;
  } else if (currency[currencyIndex].length() == 6) {
    xPosition = 226;
  }
  tft.setCursor(xPosition, 30);
  tft.print(currency[currencyIndex]);
}

String trimTrailingZeros(String str) {
  // ตรวจสอบว่ามีจุดทศนิยมใน String หรือไม่
  if (str.indexOf('.') != -1) {
    // ลบศูนย์ด้านหลังออกจนกว่าค่าจะไม่ใช่ศูนย์
    while (str.endsWith("0")) {
      str.remove(str.length() - 1);
    }
    // หากจุดทศนิยมอยู่ท้ายสุดให้ลบออกด้วย
    if (str.endsWith(".")) {
      str.remove(str.length() - 1);
    }
  }
  return str;
}

String formatWithComma(String numberStr) {
  int len = numberStr.length();
  int dotIndex = numberStr.indexOf('.'); // ตำแหน่งจุดทศนิยม (ถ้ามี)
  int start = dotIndex == -1 ? len : dotIndex; // ตำแหน่งเริ่มการแบ่งส่วนหลัก

  String formattedStr = "";
  int count = 0;

  for (int i = start - 1; i >= 0; i--) {
    if (count > 0 && count % 3 == 0) {
      formattedStr = "," + formattedStr; // เพิ่ม , ทุกๆ สามหลัก
    }
    formattedStr = numberStr[i] + formattedStr;
    count++;
  }

  if (dotIndex != -1) {
    // เพิ่มทศนิยมกลับเข้าไปในรูปแบบ
    formattedStr += numberStr.substring(dotIndex);
  }

  return formattedStr;
}






void displayPriceOnScreen(float priceTemp) {
  // priceTemp = 10000000;
  String priceStr = "";
  String unit = ""; // Unit for micro (u) or nano (n) if needed

  // ตรวจสอบว่ามีค่า 999 หรือ 998 หรือไม่ แล้วปัดขึ้น
  if (static_cast<int>(priceTemp) % 1000 == 999 || static_cast<int>(priceTemp) % 1000 == 998) {
    priceTemp = ceil(priceTemp / 1000) * 1000;
  }

  // กรณีราคาต่ำกว่า 0.000001 ให้ใช้ 'n'
  if (priceTemp < 0.000001) {
    priceTemp *= 1000000000;
    unit = "n";
  }
  // กรณีราคาต่ำกว่า 0.001 ให้ใช้ 'u'
  else if (priceTemp < 0.001) {
    priceTemp *= 1000000;
    unit = "u";
  }
  else if (priceTemp >= 1000000000) {
    priceTemp = priceTemp / 1000000.0; // หารล้าน
    unit = " M";
    priceStr = String(priceTemp, 2);
  }




  // จัดรูปแบบการแสดงราคาตามช่วงของราคา
  if (priceTemp >= 100000) {
    priceStr = String(priceTemp, 0); // ไม่แสดงทศนิยมสำหรับตัวเลขใหญ่
  } else if (priceTemp >= 10000) {
    priceStr = String(priceTemp, 1); // ไม่แสดงทศนิยมสำหรับตัวเลขใหญ่
  }  else if (priceTemp >= 1000) {
    priceStr = String(priceTemp, 2); // ไม่แสดงทศนิยมสำหรับตัวเลขใหญ่
  }  else if (priceTemp >= 100) {
    priceStr = String(priceTemp, 3); // แสดงทศนิยม 3 ตำแหน่งสำหรับตัวเลขเล็ก
  } else if (priceTemp >= 10) {
    priceStr = String(priceTemp, 4); // แสดงทศนิยม 4 ตำแหน่งสำหรับตัวเลขขนาดกลาง
  } else {
    priceStr = String(priceTemp, 5); // แสดงทศนิยม 5 ตำแหน่งสำหรับตัวเลขอื่น ๆ
  }

  priceStr = trimTrailingZeros(priceStr);
  priceStr = formatWithComma(priceStr); // ใช้ฟังก์ชันเพื่อเพิ่ม ,

  int priceWidth = priceStr.length() * 28; // Estimate the width of the price string
  Serial.print("priceStr:");
  Serial.println(priceStr);
  int textSize = 5; // ค่าเริ่มต้น
  if (priceStr.length() > 9) { // หากจำนวนตัวเลขเกิน 7 หลัก
    textSize = 4; // ลดขนาดข้อความ
    priceWidth = priceStr.length() * 23;
    Serial.println("length > 8");
  }
  //  if (priceStr.indexOf(',') != 0)
  //  {
  //    priceWidth = priceStr.length() * 28;
  //  }
  int xPosition = (320 - priceWidth) / 2; // Center the price string
  Serial.println(xPosition);

  tft.setTextSize(textSize);
  tft.setTextColor(convertColor(color[coinIndex]));
  if (priceStr.length() > 9) {
    tft.setCursor(xPosition, 115);
  }
  else
  {
    tft.setCursor(xPosition, 105);
  }

  tft.setTextSize(textSize);
  tft.print(priceStr);
  tft.setTextColor(TFT_WHITE);
  tft.setTextSize(2);
  tft.print(unit);
}

void testDisplayPriceOnScreen() {
  float price = 0.000000000001; // เริ่มต้นจากค่าน้อยที่สุด
  while (price <= 10000000000.0) { // ถึงค่ามากที่สุด
    Serial.print("Testing price: ");
    Serial.println(price);
    displayPriceOnScreen(price); // เรียกฟังก์ชันที่ต้องการทดสอบ
    price *= 10; // เพิ่มค่าโดยการคูณ 10 ในแต่ละรอบ
    delay(1500); // เพิ่มเวลารอเล็กน้อยเพื่อดูผล
    tft.fillScreen(TFT_BLACK);
  }
}


void displayCenteredPrice(float priceTemp, bool isLow) {
  String priceStr = "";
  int priceLength;

  // ตรวจสอบว่ามีค่า 999 หรือ 998 หรือไม่ แล้วปัดขึ้น
  if (static_cast<int>(priceTemp) % 1000 == 999 || static_cast<int>(priceTemp) % 1000 == 998) {
    priceTemp = ceil(priceTemp / 1000) * 1000;
  }

  // ==== สเกลแบบเดียวกับ displayPriceOnScreen แต่ไม่ใช้ suffix ====
  if (priceTemp < 0.000001) {
    // เดิมหน่วย n (nano) -> คูณ 1e9
    priceTemp *= 1000000000.0;
  } else if (priceTemp < 0.001) {
    // เดิมหน่วย u (micro) -> คูณ 1e6
    priceTemp *= 1000000.0;
  } else if (priceTemp >= 1000000000.0) {
    // เดิมหน่วย M (ล้าน) -> หาร 1e6
    priceTemp /= 1000000.0;
  }

  // ==== รูปแบบทศนิยมตามช่วงราคา เหมือนตัวหลัก ====
  if (priceTemp >= 100000) {
    priceStr = String(priceTemp, 0);      // ไม่เอาทศนิยม
  } else if (priceTemp >= 10000) {
    priceStr = String(priceTemp, 1);
  } else if (priceTemp >= 1000) {
    priceStr = String(priceTemp, 2);
  } else if (priceTemp >= 100) {
    priceStr = String(priceTemp, 3);
  } else if (priceTemp >= 10) {
    priceStr = String(priceTemp, 4);
  } else {
    priceStr = String(priceTemp, 5);
  }

  // ตัด 0 ท้าย ๆ ถ้ามี
  priceStr = trimTrailingZeros(priceStr);

  priceLength = priceStr.length();

  int startXRange = isLow ? 0 : 195; // Start of x range for low or high
  int endXRange = isLow ? 120 : 315; // End of x range for low or high
  int rangeWidth = endXRange - startXRange; // Width of the x range

  int textWidth = priceLength * 10; // Estimated width of the text
  int xPosition = startXRange + (rangeWidth - textWidth) / 2; // Calculate starting x position to center text

  int yPosition = 200; // Y position

  tft.setTextColor(TFT_WHITE);
  tft.setTextSize(2);
  tft.setCursor(xPosition, yPosition);
  tft.print(priceStr);
}

void displayPriceDetails() {
  char formattedPrice[16]; // Buffer for formatted price
  String unit = "";
  float currentPriceChange = displayChange[coinIndex]; // Use the existing priceChange array

  // Scaling the priceChange based on its magnitude
  if (currentPriceChange > 0 && currentPriceChange < 0.000001) {
    currentPriceChange *= 1000000000;
    unit = "n";
  } else if (currentPriceChange > 0 && currentPriceChange < 0.001) {
    currentPriceChange *= 1000000;
    unit = "u";
  } else if (currentPriceChange < 0 && currentPriceChange > -0.000001) {
    currentPriceChange *= 1000000000;
    unit = "n";
  } else if (currentPriceChange < 0 && currentPriceChange > -0.001) {
    currentPriceChange *= 1000000;
    unit = "u";
  }

  // Determine the number of decimals based on scaled priceChange
  int decimals = (currentPriceChange >= 100000) ? 0 :
                 (currentPriceChange >= 10000) ? 1 :
                 (currentPriceChange >= 1000) ? 2 :
                 (currentPriceChange >= 100) ? 3 :
                 (currentPriceChange >= 10) ? 4 : 2;

  snprintf(formattedPrice, sizeof(formattedPrice), "%s%.*f",
           (currentPriceChange >= 0 ? "+" : ""), decimals, currentPriceChange);

  // Remove trailing zeros from decimals
  char* dotPointer = strchr(formattedPrice, '.');
  if (dotPointer != NULL) {
    char* endPointer = dotPointer;
    while (*++endPointer);
    while (*--endPointer == '0');
    if (*endPointer == '.') *endPointer = '\0';
    else *(endPointer + 1) = '\0';
  }

  strcat(formattedPrice, unit.c_str());

  int yPosition = 85; // Base Y position for the price
  int xPosition = 10;

  tft.setTextSize(1);
  tft.setCursor(xPosition, yPosition);
  tft.setTextColor(currentPriceChange >= 0 ? TFT_GREEN : TFT_RED);
  tft.print(formattedPrice); // Display the formatted price
}


void displayPriceChangePercent() {
  char changeBuffer[32];
  snprintf(changeBuffer, sizeof(changeBuffer), "%s%.2f%%", (displayChangePercent[coinIndex] >= 0 ? "+" : ""), displayChangePercent[coinIndex]);

  int yPosition = 85;
  int screenMaxWidth = 310;
  int textWidth = strlen(changeBuffer) * 6;
  int xPosition = screenMaxWidth - textWidth;

  tft.setTextSize(1);
  tft.setCursor(xPosition, yPosition);
  tft.setTextColor(priceChangePercent[coinIndex] >= 0 ? TFT_GREEN : TFT_RED);
  tft.print(changeBuffer);
}

void displayHighLowPrice() {

  if (coin[coinIndex] != "ZENT")
  {
    // แสดงข้อความ "Lowest"
    tft.setTextColor(TFT_GREEN);
    tft.setTextSize(2);
    tft.setCursor(30, 180); // ตำแหน่งซ้ายบนของราคา Low
    tft.print("Lowest");

    // แสดงข้อความ "Highest"
    tft.setTextColor(TFT_RED);
    tft.setCursor(223, 180); // ตำแหน่งขวาบนของราคา High
    tft.print("Highest");
  }



  displayCenteredPrice(displayLowPrice[coinIndex], true);
  displayCenteredPrice(displayHighPrice[coinIndex], false);
  displayPriceDetails();
  displayPriceChangePercent();
  tft.setTextColor(TFT_WHITE);
}

void showLogoAtTopMiddle() {
  String imagePath = "/" + firmwareVersion + "/system/image/coin" + String(coinIndex + 1) + ".jpg";
  if (coin[coinIndex] == "USD" && coinType == 6)
  {
    imagePath = "/" + firmwareVersion + "/system/image/usdt.jpg";
  }
  int16_t xpos = (tft.width() - 48) / 2;
  int16_t ypos = 10;
  if (coin[coinIndex] == "ZENT")
  {

  }
  else
  {
    clampBuzzerForPage2Draw();
    tft.fillRect(xpos, ypos, 48, 48, TFT_BLACK);
#if PAGE2_BEEP_TEST_SKIP_LOGO_SD
    Serial.println("[BEEP TEST] Skip page2 top logo JPG draw");
#else
    drawSdJpeg(imagePath.c_str(), xpos, ypos);
#endif
    clampBuzzerForPage2Draw();
  }

}

void showLogoAtBottomMiddle() {
  String imagePath = "/" + firmwareVersion + "/system/image/logo.jpg";

  int16_t xpos = (tft.width() - 32) / 2;
  int16_t ypos = 175;

  clampBuzzerForPage2Draw();
  tft.fillRect(xpos, ypos, 32, 32, TFT_BLACK);
#if PAGE2_BEEP_TEST_SKIP_LOGO_SD
  Serial.println("[BEEP TEST] Skip page2 bottom logo JPG draw");
#else
  drawSdJpeg(imagePath.c_str(), xpos, ypos);
#endif
  clampBuzzerForPage2Draw();


}

void displayGoldOnScreen() {
  last_bar_sell = bar_sell;
  last_bar_buy = bar_buy;
  last_jewelry_sell = jewelry_sell;
  last_jewelry_buy = jewelry_buy;

  stopPriceTask();
  delay(1000);
  displayBackgroundImageForPage2();
  displayCoinOnScreen();

  int display_bar_sell = bar_sell;
  int display_bar_buy = bar_buy;
  int display_jewelry_sell = jewelry_sell;
  int display_jewelry_buy = jewelry_buy;

  // แปลงหน่วยถ้า currency ไม่ใช่ THB
  if (currencyIndex == 0) { // USD
    float rate = exRate[coinIndex][currencyIndex];
    display_bar_sell = static_cast<int>(bar_sell * rate);
    display_bar_buy = static_cast<int>(bar_buy * rate);
    display_jewelry_sell = static_cast<int>(jewelry_sell * rate);
    display_jewelry_buy = static_cast<int>(jewelry_buy * rate);
  }

  tft.setTextColor(TFT_ORANGE);
  tft.setTextSize(2);
  tft.setCursor(26, 130);
  tft.print(display_bar_sell);
  tft.setCursor(26, 185);
  tft.print(display_bar_buy);
  tft.setCursor(210, 130);
  tft.print(display_jewelry_sell);
  tft.setCursor(210, 185);
  tft.print(display_jewelry_buy);
  tft.setTextColor(TFT_WHITE);
  delay(1500);

  startPriceTask();
}


void showPage2() {
  tft.setTextColor(TFT_WHITE);

  if (coin[coinIndex] == "JBC" && currencyIndex == 0) {
    if (lowPrice[coinIndex] == 0 && highPrice[coinIndex] == 0) {
      lowPrice[coinIndex] = price[coinIndex];
      highPrice[coinIndex] = price[coinIndex];
    }
    if (price[coinIndex] < lowPrice[coinIndex]) {
      lowPrice[coinIndex] = price[coinIndex];
    }
    if (price[coinIndex] > highPrice[coinIndex]) {
      highPrice[coinIndex] = price[coinIndex];
    }
  } else if (coin[coinIndex] == "JBC" && currencyIndex == 1) {
    if (lowPrice[coinIndex] == 0 && highPrice[coinIndex] == 0) {
      lowPrice[coinIndex] = price[coinIndex];
      highPrice[coinIndex] = price[coinIndex];
    }
    if (price[coinIndex] < lowPrice[coinIndex]) {
      lowPrice[coinIndex] = price[coinIndex];
    } else if (price[coinIndex] > highPrice[coinIndex]) {
      highPrice[coinIndex] = price[coinIndex];
    }
  }

  if (coin[coinIndex] == "GOLD") {
    if (bar_sell != last_bar_sell || bar_buy != last_bar_buy || jewelry_sell != last_jewelry_sell || jewelry_buy != last_jewelry_buy) {
      displayGoldOnScreen();
    }
  }  else {
    if (coin[coinIndex] == "ZENT")
    {
      displayBackgroundImageForPage2();
    }
    else if (coin[coinIndex] == "JBC")
    {
      showLogoAtTopMiddle();
      showLogoAtBottomMiddle();
      tft.fillRect(0, 80, 320, 80, TFT_BLACK);
      tft.fillRect(0, 200, 110, 30, TFT_BLACK);
      tft.fillRect(210, 200, 110, 30, TFT_BLACK);
    }
    else
    {
      tft.fillRect(0, 80, 320, 80, TFT_BLACK);
      tft.fillRect(0, 200, 110, 30, TFT_BLACK);
      tft.fillRect(210, 200, 110, 30, TFT_BLACK);
    }

    displayPriceOnScreen(displayPrice[coinIndex]);

    displayHighLowPrice();


    displayCoinOnScreen();
    updateTime2();
  }
}
