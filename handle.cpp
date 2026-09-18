#include <Arduino.h>
#include "GlobalModule.h"
#include "GlobalParameter.h"
#include <XPT2046_Bitbang.h>
#include "cc_littlefs.h"
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <FS.h>
#include <Math.h>
#include "page1.h"
#include "page2.h"
#include "page3.h"
#include "page6.h"
#include "page8.h"
#include "page9.h"
#include "page10.h"
#include "page11.h"
#include "page12.h"
#include "page13.h"
#include "page14.h"
#include "page15.h"
#include "page16.h"
#include "page17.h"
#include "page18.h"
#include "page19.h"
#include "page20.h"
#include "page21.h"
#include "page22.h"
#include "page23.h"
#include "page24.h"
#include "page25.h"
#include "page26.h"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <string>
#include <iostream>
#include "qrcode_gen.h"
#include "GlobalFunction.h"

void handleTouchInput(TouchPoint touch);
TouchPoint checkTouchScreen();

// local
//  กำหนดค่าคงที่สำหรับ debounce delay (เป็นมิลลิวินาที)
const unsigned long DEBOUNCE_DELAY = 50; // ตัวอย่าง: 50 มิลลิวินาที
int prevPage = 0;
extern int pageSequenceIndex;

static bool isDynamicPageTouchEnabled()
{
  return pageMode == "dynamic" && serverOn == false;
}

static void goToCDCPageFromTouch()
{
  Serial.println("Touch go to page26");
  stopPriceTask();
  pageIndex = 26;
  pageSequenceIndex = 3;
  tft.fillScreen(TFT_BLACK);
  showPage26();
  lastUpdateTime = millis();
  pageChangeTime = millis();
  delay(150);
}

static void requestDynamicNextPage()
{
  Serial.println("Next");
  pageChangeTime = millis() - 90000;
}

void enterLowPowerMode()
{
  // Example: Dimming the backlight
  analogWrite(21, 0); // Set to a lower brightness
}

void exitLowPowerMode()
{
  int pwmValue = map(brightnessLevel, 0, 100, 0, 255);
  analogWrite(21, pwmValue);
}

TouchPoint checkTouchScreen()
{
  static unsigned long lastDebounceTime = 0; // เวลาที่ใช้ในการทำ debounce ล่าสุด
  static bool debounceActive = false;        // สถานะการทำงานของ debounce
  static unsigned long lastTouchTime = 0;    // เวลาที่ตรวจพบการสัมผัสล่าสุด
  static TouchPoint lastTouch = {0, 0, 0};   // จุดสัมผัสล่าสุด
  static TouchPoint maxZTouch = {0, 0, 0};   // จุดสัมผัสที่มีค่า zRaw สูงสุด
  static bool wasTouched = false;            // สถานะการสัมผัส

  TouchPoint t = ts.getTouch();
  unsigned long currentTime = millis();

  // Debounce logic for initial touch
  if (t.zRaw > 280)
  {
    if (!debounceActive)
    {
      lastDebounceTime = currentTime;
      debounceActive = true;
      return {0, 0, 0}; // Early exit, ignore touch for debounce period
    }
    else if (currentTime - lastDebounceTime < DEBOUNCE_DELAY)
    {
      return {0, 0, 0}; // Still in debounce period, ignore touch
    }
    // Touch is valid, process it
    debounceActive = false; // Reset debounce on valid touch
    wasTouched = true;
    if (t.zRaw > maxZTouch.zRaw)
    {
      maxZTouch = t; // Update maxZTouch if current touch has higher zRaw
    }
    lastTouch = t; // Update last touch point
    lastTouchTime = currentTime;
    return {0, 0, 0}; // Return invalid touch point during touch
  }
  else
  {
    // No current touch detected
    if (wasTouched)
    {
      // A touch was previously detected, handle touch release
      wasTouched = false;
      TouchPoint highestZTouch = maxZTouch; // Use maxZTouch as the touch with highest zRaw
      lastTouch = {0, 0, 0};
      maxZTouch = {0, 0, 0}; // Reset maxZTouch for next detection
      if (isLowPowerMode)
      {
        if (pageMode != "dynamic")
        {
          Serial.println("NormalMode");
          pageIndex = 2;
          isLowPowerMode = false;
          brightnessLevel = 100;
          exitLowPowerMode();
          gotopage2();
          delay(500);
          return highestZTouch; // Return the highest zRaw touch point
        }
        else
        {
          Serial.println("Next");
          pageChangeTime = millis() - 90000;
          Serial.println(pageChangeTime);
          return highestZTouch;
        }
      }
      else
      {
        Serial.println("press");
        return highestZTouch; // Return the highest zRaw touch point
      }
    }
    //    if (!isLowPowerMode && currentTime - lastTouchTime > timeout && timeout > 0)
    //    {
    //      if (sdStatus == true && (pageIndex == 1 || pageIndex == 2))
    //      {
    //        Serial.println("SlideMode");
    //        stopPriceTask();
    //        pageIndex = 7;
    //        isLowPowerMode = true;
    //      }
    //      else if (pageIndex == 1 || pageIndex == 2)
    //      {
    //
    //        Serial.println("LowPowerMode");
    //        enterLowPowerMode(); // Enter low power mode if timeout exceeded
    //        isLowPowerMode = true;
    //      }
    //    }
  }

  // If no touch or release detected, return an invalid touch point
  return {0, 0, 0};
}

void handleTouchInput(TouchPoint touch)
{
  if (touch.zRaw > 250)
  {
//    struct BuzzerSettleGuard {
//      ~BuzzerSettleGuard() {
//        settleBuzzerAfterUiEvent();
//      }
//    } buzzerSettleGuard;

    Serial.print("oldPage:");
    Serial.println(pageIndex);
    size_t heapSize = ESP.getFreeHeap();
    Serial.println("heap:");
    Serial.println(heapSize);

    // กำหนดขนาดหน้าจอ
    int screenWidth = 320;  // แทนด้วยความกว้างของหน้าจอ
    int screenHeight = 240; // แทนด้วยความสูงของหน้าจอ

    // หากจอหมุน 180 องศา ต้องสลับพิกัด
    int adjustedX = screenWidth - touch.x;
    int adjustedY = screenHeight - touch.y;

    if (rotate180 == true)
    {
      // ใช้ adjustedX และ adjustedY แทน touch.x และ touch.y
      touch.x = adjustedX;
      touch.y = adjustedY;
    }

    if (pageIndex == 1)
    {

      if (touch.x >= 10 && touch.x <= 100 && touch.y >= 10 && touch.y <= 100) // Profile for Profile Settings
      {
        //        pageIndex = 9 ;
        //        menuId = 5 ;
        //        showPage9();
        //        delay(500);
      }
      else
      {
        if (isDynamicPageTouchEnabled() && (pageIndex == 1 || pageIndex == 2 || pageIndex == 7 || pageIndex == 26))
        {
          requestDynamicNextPage();
          // pageChangeTime = millis();
        }
        else
        {
          Serial.println("Static go to page 2");
          pageChangeTime = millis() - 90000;
          gotopage2();
        }
      }
    }
    else if (pageIndex == 2)
    {
      if (touch.x >= 0 && touch.x <= 130 && touch.y >= 10 && touch.y <= 80) // coin button
      {
        Serial.println("Change Coin");
        silenceBuzzer(); // BEEP TEST: clamp passive buzzer before stopping task/redrawing page 2.
        while (waitforprice == true) delay(10);
        Serial.println("end task");
        stopPriceTask();
        silenceBuzzer();
        delay(30);
        Serial.print("coinType:");
        Serial.println(coinType);
        Serial.print("coin[coinIndex]:");
        Serial.println(coin[coinIndex]);
        Serial.print("maxCoin:");
        Serial.println(maxCoin);


        // หากเหรียญเป็น USD และอยู่ในหมวด coinType == 6 → สลับ currency เป็น THB
        if (coinType == 6 && coin[coinIndex] == "USD") {
          currencyIndex = 1;
          // ⬅️ เปลี่ยนเหรียญ
          coinIndex = (coinIndex < maxCoin - 1) ? coinIndex + 1 : 0;
          coinType = type[coinIndex].toInt();
          Serial.println("🔄 ปรับ currency เป็น THB อัตโนมัติเมื่อเลือก USD");
        }
        else
        {
          // ⬅️ เปลี่ยนเหรียญ
          coinIndex = (coinIndex < maxCoin - 1) ? coinIndex + 1 : 0;
          coinType = type[coinIndex].toInt();
        }
        // หากเหรียญเป็น USD และอยู่ในหมวด coinType == 6 → สลับ currency เป็น THB
        if (coinType == 6 && coin[coinIndex] == "USD") {
          currencyIndex = 1; //change to thb
          Serial.println("🔄 ปรับ currency เป็น THB อัตโนมัติเมื่อเลือก USD");
        }

        if (coinType != 6)
        {
          // คำนวณค่าที่ต้องแสดงผลใหม่
          displayPrice[coinIndex]         = price[coinIndex]         * exRate[coinIndex][currencyIndex];
          displayHighPrice[coinIndex]     = highPrice[coinIndex]     * exRate[coinIndex][currencyIndex];
          displayLowPrice[coinIndex]      = lowPrice[coinIndex]      * exRate[coinIndex][currencyIndex];
          displayChange[coinIndex]        = priceChange[coinIndex]   * exRate[coinIndex][currencyIndex];
          displayChangePercent[coinIndex] = priceChangePercent[coinIndex]; // ไม่ต้องแปลง
        }
        else
        {
          // คำนวณค่าที่ต้องแสดงผลใหม่
          displayPrice[coinIndex]         = price[coinIndex]       ;
          displayHighPrice[coinIndex]     = highPrice[coinIndex]    ;
          displayLowPrice[coinIndex]      = lowPrice[coinIndex]     ;
          displayChange[coinIndex]        = priceChange[coinIndex]  ;
          displayChangePercent[coinIndex] = priceChangePercent[coinIndex]; // ไม่ต้องแปลง
        }

        // 🎨 แสดงผลหน้าจอ
        silenceBuzzer();
        if (coin[coinIndex] == "GOLD") {
          last_bar_sell = 0;
          last_bar_buy = 0;
          last_jewelry_sell = 0;
          last_jewelry_buy = 0;
           if (coin[coinIndex] == "GOLD"  && currencyIndex == 0)
          {
            currencyIndex = 1;
            coinType = type[coinIndex].toInt();
            return ;
          }
          displayGoldOnScreen();
         
        } else {
          displayBackgroundImageForPage2();
          showLogoAtTopMiddle();
          if (coin[coinIndex] != "ZENT")
          {
            showLogoAtBottomMiddle();
          }
          displayCoinOnScreen();
          displayPriceOnScreen(displayPrice[coinIndex]);
          displayHighLowPrice();
        }
        silenceBuzzer();

        delay(100);
        waitforclick = true;
        updateClickInterval = millis();
        pageChangeTime = millis();
      }

      else if (touch.x >= 136 && touch.x <= 184 && touch.y >= 20 && touch.y <= 80) // set coin
      {
        while (waitforprice == true)
        {
          delay(10);
        }
        stopPriceTask();
        pageIndex = 5;
        Serial.print("page : ");
        Serial.println(pageIndex);
      }
      else if (touch.x >= 130 && touch.x <= 190 && touch.y >= 150 && touch.y <= 210) // set button
      {
        //        stopPriceTask();
        //        tft.fillScreen(TFT_BLACK); // Clear the screen
        //        pageIndex = 3;
        //        showPage3();
        //        delay(150);
        stopPriceTask();
        tft.fillScreen(TFT_BLACK); // Clear the screen
        pageIndex = 9;
        menuId = 0;
        showPage9();
        delay(150);
      }
      else if (touch.x >= 186 && touch.x <= 320 && touch.y >= 10 && touch.y <= 80) // currency button
      {
        Serial.println("Change Currency");
        silenceBuzzer(); // BEEP TEST: same redraw path as coin change.
        if ((coin[coinIndex] == "USD" || coin[coinIndex] == "USDT") && currencyIndex == 1)
        {
          return ;
        }
        if (coin[coinIndex] == "GOLD"  && currencyIndex == 1)
        {
          return ;
        }
        while (waitforprice == true) delay(10);
        stopPriceTask();
        silenceBuzzer();

        // 🔁 เปลี่ยนสกุลเงิน
        currencyIndex = (currencyIndex == 0) ? 1 : 0;

        coinType = type[coinIndex].toInt();
        if (coinType == 3)
        {
          if (currencyIndex == 0)
          {
            displayPrice[coinIndex] /= USDTHBRate ;
            displayHighPrice[coinIndex]     /= USDTHBRate;
            displayLowPrice[coinIndex]       /= USDTHBRate;
            displayChange[coinIndex]        /= USDTHBRate;
            displayChangePercent[coinIndex] = priceChangePercent[coinIndex]; // ไม่ต้องแปลง
          }
          else if (currencyIndex == 1)
          {
            displayPrice[coinIndex] *= USDTHBRate ;
            displayHighPrice[coinIndex]     *= USDTHBRate ;
            displayLowPrice[coinIndex]     *= USDTHBRate ;
            displayChange[coinIndex]       *= USDTHBRate ;
            displayChangePercent[coinIndex] = priceChangePercent[coinIndex]; // ไม่ต้องแปลง
          }
        }
        else
        {
          // 💹 คำนวณค่าที่แสดงใหม่
          Serial.println("updateDisplayValues");
          updateDisplayValues();
        }


        // ✅ แสดงผล
        silenceBuzzer();
        displayBackgroundImageForPage2();
        showLogoAtTopMiddle();
        if (coin[coinIndex] == "GOLD") {
          last_bar_sell = 0;
          last_bar_buy = 0;
          last_jewelry_sell = 0;
          last_jewelry_buy = 0;
          displayGoldOnScreen();
        } else {
          if (coin[coinIndex] != "ZENT")
          {
            showLogoAtBottomMiddle();
          }
          displayCoinOnScreen();
          Serial.println("updateDisplayValues");
          displayPriceOnScreen(displayPrice[coinIndex]);
          displayHighLowPrice();
        }
        silenceBuzzer();
        updateExchangeRate();
        delay(10);
        waitforclick = true;
        updateClickInterval = millis();
        pageChangeTime = millis();
      }


      else
      {
        if (isDynamicPageTouchEnabled() && cdcToggle == true)
        {
          goToCDCPageFromTouch();
        }
        else
        {
          Serial.println("go to page1");
          pageChangeTime = millis();
          gotopage1();
        }
      }
    }
    else if (pageIndex == 3)
    {
      if (touch.x >= 50 && touch.x <= 100 && touch.y >= 200 && touch.y <= 240) // back button
      {
        gotopage2();
      }
      // Brightness Down (-)
      else if (touch.x >= 240 && touch.x <= 300 && touch.y >= 85 && touch.y <= 125)
      {
        // Decrease brightness level
        Serial.println("Decrease Brightness");
        // Implement brightness decrease logic here
        decreaseBrightness(); // Decrease brightness level
        delay(600);
      }
      // Brightness Up (+)
      else if (touch.x >= 200 && touch.x <= 240 && touch.y >= 85 && touch.y <= 125)
      {
        // Increase brightness level
        Serial.println("Increase Brightness");
        // Implement brightness increase logic here
        increaseBrightness(); // Increase brightness level
        delay(600);
      }
      // Interval Down (-)
      else if (touch.x >= 240 && touch.x <= 300 && touch.y >= 125 && touch.y <= 165)
      {
        // Decrease interval
        Serial.println("Decrease Interval");
        // Implement interval decrease logic here
        decreaseInterval();
        delay(600);
      }
      // Interval Up (+)
      else if (touch.x >= 200 && touch.x <= 240 && touch.y >= 125 && touch.y <= 165)
      {
        // Increase interval
        Serial.println("Increase Interval");
        // Implement interval increase logic here
        increaseInterval();
        delay(600);
      }
      // Timeout Down (-)
      else if (touch.x >= 240 && touch.x <= 300 && touch.y >= 165 && touch.y <= 205)
      {
        // Decrease timeout
        Serial.println("Decrease Timeout");
        // Implement timeout decrease logic here
        decreaseTimeout();
        delay(600);
      }
      // Timeout Up (+)
      else if (touch.x >= 200 && touch.x <= 240 && touch.y >= 165 && touch.y <= 205)
      {
        // Increase timeout
        Serial.println("Increase Timeout");
        // Implement timeout increase logic here
        increaseTimeout();
        delay(600);
      }
    }
    else if (pageIndex == 4)
    {
      if (touch.x >= 40 && touch.x <= 110 && touch.y >= 180 && touch.y <= 240) // back button
      {
        ESP.restart();
      }
    }
    else if (pageIndex == 5)
    {
      if (touch.x >= 40 && touch.x <= 110 && touch.y >= 180 && touch.y <= 240) // back button
      {
        ESP.restart();
      }
    }
    else if (pageIndex == 6)
    {
      gotopage1();
    }
    else if (pageIndex == 7)
    {
      if (isDynamicPageTouchEnabled() && (pageIndex == 1 || pageIndex == 2 || pageIndex == 7 || pageIndex == 26))
      {
        requestDynamicNextPage();
        Serial.println(pageChangeTime);
        // pageChangeTime = millis();
      }
      else
      {
        gotopage2();
      }
    }
    else if (pageIndex == 26)
    {
      if (handlePage26Touch(touch.x, touch.y))
      {
        return;
      }

      if (isDynamicPageTouchEnabled())
      {
        pageSequenceIndex = 3;
        requestDynamicNextPage();
      }
      else
      {
        gotopage1();
      }
    }
    else if (pageIndex == 8)
    { // Game Dino
      // Left Arrow
      if (touch.x >= 0 && touch.x <= 80 && touch.y >= 0 && touch.y <= 50)
      {
        // Code for Left Arrow  // Back to Setting
        pageIndex = 9;
        menuId = 0;
        showPage9();
        delay(500);
      }
      // Righ
    }
    else if (pageIndex == 9)
    { // All Menus
      if (menuId == 0) // setting 1
      {
        // Left Arrow
        if (touch.x >= 0 && touch.x <= 80 && touch.y >= 0 && touch.y <= 50)
        {
          // Code for Left Arrow  // Back to Page2
          pageIndex = 2;
          gotopage2();
          delay(500);
        }
        // Right Arrow
        else if (touch.x >= 240 && touch.x <= 320 && touch.y >= 0 && touch.y <= 50)
        {
          // Code for Right Arrow // Next Settings
          pageIndex = 9;
          menuId = 1;
          showPage9();
          delay(500);
        }
        // Button 1 (NOTI)
        else if (touch.x >= 18 && touch.x <= 18 + 140 && touch.y >= 50 && touch.y <= 48 + 90)
        {
          // Code for NOTI button // goto Page 12 NOTI
          pageIndex = 12;
          showPage12();
          delay(500);
          //          prevPage = 9;
          //          pageIndex = 23;
          //          showPage23();
          //          delay(500);
        }
        // Button 2 (NEWS)
        else if (touch.x >= 18 && touch.x <= 18 + 140 && touch.y >= 140 && touch.y <= 140 + 90)
        {
          // Code for NOTI button // goto Page 12 Noti
          //          pageIndex = 19;
          //          showPage19();
          //          delay(500);
          prevPage = 9;
          pageIndex = 23;
          showPage23();
          delay(500);
        }
        // Button 3 (HORO)
        else if (touch.x >= tft.width() - 18 - 140 && touch.x <= tft.width() - 18 && touch.y >= 50 && touch.y <= 48 + 90)
        {
          // Code for HORO button
          prevPage = 9;
          pageIndex = 23;
          showPage23();
          delay(500);
        }
        // Button 4 (GAME)
        else if (touch.x >= tft.width() - 18 - 140 && touch.x <= tft.width() - 18 && touch.y >= 140 && touch.y <= 140 + 90)
        {
          // Code for NEWs button
          pageIndex = 8;
          setupPage8();
          // showPage8();
          Serial.println("Open Game Page");
        }
      }
      else if (menuId == 1) // Settings 2
      {
        // Left Arrow
        if (touch.x >= 0 && touch.x <= 80 && touch.y >= 0 && touch.y <= 50)
        {
          pageIndex = 9;
          menuId = 0;
          showPage9();
          delay(500);
        }
        // Right Arrow
        else if (touch.x >= 240 && touch.x <= 320 && touch.y >= 0 && touch.y <= 50)
        {
          pageIndex = 9;
          menuId = 2;
          showPage9();
          delay(500);
        }
        // Button 1 (TOKEN)
        else if (touch.x >= 18 && touch.x <= 18 + 140 && touch.y >= 50 && touch.y <= 48 + 90)
        {

          pageIndex = 24;
          //showPage24(); // token page
          //delay(500);
          //          prevPage = 9;
          //          pageIndex = 23;
          //          showPage23();
          //          delay(500);
        }
        // Button 2 (PLAN)
        else if (touch.x >= 18 && touch.x <= 18 + 140 && touch.y >= 140 && touch.y <= 140 + 90)
        {
          prevPage = 9;
          pageIndex = 23;
          showPage23();
          delay(500);
        }
        // Button 3 (MAIL)
        else if (touch.x >= tft.width() - 18 - 140 && touch.x <= tft.width() - 18 && touch.y >= 50 && touch.y <= 48 + 90)
        {
          prevPage = 9;
          pageIndex = 23;
          showPage23();
          delay(500);
        }
        // Button 4 (ACCOUNT)
        else if (touch.x >= tft.width() - 18 - 140 && touch.x <= tft.width() - 18 && touch.y >= 140 && touch.y <= 140 + 90)
        {
          prevPage = 9;
          pageIndex = 23;
          showPage23();
          delay(500);
        }
      }
      else if (menuId == 2) // Settings 3
      {
        // Left Arrow to Setting 2
        if (touch.x >= 0 && touch.x <= 80 && touch.y >= 0 && touch.y <= 50)
        {
          pageIndex = 9;
          menuId = 1;
          showPage9();
          delay(500);
        }
        // Right Arrow to Setting 4
        else if (touch.x >= 240 && touch.x <= 320 && touch.y >= 0 && touch.y <= 50)
        {
          pageIndex = 9;
          menuId = 3;
          showPage9();
          delay(500);
        }
        // Button 1 (VOTE)
        else if (touch.x >= 18 && touch.x <= 18 + 140 && touch.y >= 50 && touch.y <= 48 + 90)
        {
          prevPage = 9;
          pageIndex = 23;
          showPage23();
          delay(500);
        }
        // Button 2 (BILL)
        else if (touch.x >= 18 && touch.x <= 18 + 140 && touch.y >= 140 && touch.y <= 140 + 90)
        {
          prevPage = 9;
          pageIndex = 23;
          showPage23();
          delay(500);
        }
        // Button 3 (OTA)
        else if (touch.x >= tft.width() - 18 - 140 && touch.x <= tft.width() - 18 && touch.y >= 50 && touch.y <= 48 + 90)
        {
          prevPage = 9;
          pageIndex = 23;
          showPage23();
          delay(500);
        }
        // Button 4 (REFER)
        else if (touch.x >= tft.width() - 18 - 140 && touch.x <= tft.width() - 18 && touch.y >= 140 && touch.y <= 140 + 90)
        {
          prevPage = 9;
          pageIndex = 23;
          showPage23();
          delay(500);
        }
      }
      else if (menuId == 3) // Settings 4
      {
        // Left Arrow to Setting 3
        if (touch.x >= 0 && touch.x <= 80 && touch.y >= 0 && touch.y <= 50)
        {
          pageIndex = 9;
          menuId = 2;
          showPage9();
          delay(500);
        }
        // Right Arrow
        else if (touch.x >= 240 && touch.x <= 320 && touch.y >= 0 && touch.y <= 50)
        {
        }
        // Button 1 (MANUAL)
        else if (touch.x >= 18 && touch.x <= 18 + 140 && touch.y >= 50 && touch.y <= 48 + 90)
        {
          prevPage = 9;
          pageIndex = 23;
          showPage23();
          delay(500);
        }
        // Button 2 (SHOP)
        else if (touch.x >= 18 && touch.x <= 18 + 140 && touch.y >= 140 && touch.y <= 140 + 90)
        {
          prevPage = 9;
          pageIndex = 23;
          showPage23();
          delay(500);
        }
        // Button 3 (FAQ)
        else if (touch.x >= tft.width() - 18 - 140 && touch.x <= tft.width() - 18 && touch.y >= 50 && touch.y <= 48 + 90)
        {
          prevPage = 9;
          pageIndex = 23;
          showPage23();
          delay(500);
        }
        // Button 4 (SYSTEM)
        else if (touch.x >= tft.width() - 18 - 140 && touch.x <= tft.width() - 18 && touch.y >= 140 && touch.y <= 140 + 90)
        {

          pageIndex = 3;
          showPage3();
          delay(500);
        }
      }
      else if (menuId == 4) // pp usdt sat cct
      {
        // Left Arrow
        if (touch.x >= 0 && touch.x <= 80 && touch.y >= 0 && touch.y <= 50)
        {
          // Code for Left Arrow  // Back to Settihg4
          pageIndex = 9;
          menuId = 9;
          showPage9();
          delay(500);
        }
        // Button 1 (PROMPTPAY // STRIPE)
        else if (touch.x >= 18 && touch.x <= 18 + 140 && touch.y >= 50 && touch.y <= 48 + 90)
        {
          //gen qr
          payPrice = stripePrice;

          pageIndex = 17;
          currentWallet = 0; //{"PROMPTPAY", "USDT", "SAT", "CCT"}
          coinReceive = 0;   // 0 = shop , 1 = user
          amounts = payPrice;
          coinInput = amounts;
          stripeAmount = amounts ;
          coinOutput = selectedPrice;
          pointAmount = (int)coinOutput ;
          showPage17();
          delay(500);
        }
        // Button 2 (USDT)
        else if (touch.x >= 18 && touch.x <= 18 + 140 && touch.y >= 140 && touch.y <= 140 + 90)
        {
          // การกระทำเมื่อกดปุ่ม RECEIVE
          payPrice = usdtPrice ;

          pageIndex = 17;
          currentWallet = 1; //{"PROMPTPAY", "USDT", "SAT", "CCT"}
          coinReceive = 0;   // 0 = shop , 1 = user

          amounts = payPrice;
          usdtAmount = amounts ;
          coinInput = amounts;
          coinOutput = selectedPrice;
          pointAmount = (int)coinOutput ;
          showPage17();
          delay(500);
        }
        // Button 3 (SAT)
        else if (touch.x >= tft.width() - 18 - 140 && touch.x <= tft.width() - 18 && touch.y >= 50 && touch.y <= 48 + 90)
        {
          payPrice = satoshiPrice;
          //payPrice = 1;
          pageIndex = 17;
          currentWallet = 2; //{"PROMPTPAY", "USDT", "SAT", "CCT"}
          coinReceive = 0;   // 0 = shop , 1 = user
          amounts = payPrice;
          satAmount = amounts ;
          coinInput = amounts;
          coinOutput = selectedPrice;
          pointAmount = (int)coinOutput ;
          showPage17();
          delay(500);
        }
        // Button 4 (CCT)
        else if (touch.x >= tft.width() - 18 - 140 && touch.x <= tft.width() - 18 && touch.y >= 140 && touch.y <= 140 + 90)
        {
          payPrice = cctPrice;
          pageIndex = 17;
          currentWallet = 3; //{"PROMPTPAY", "USDT", "SAT", "CCT"}
          coinReceive = 0;   // 0 = shop , 1 = user
          amounts = payPrice;
          cctAmount = amounts ;
          coinInput = amounts;
          coinOutput = selectedPrice;
          pointAmount = (int)coinOutput ;
          showPage17();
          delay(500);
        }
      }
      else if (menuId == 5)
      {
        // Left Arrow to Setting 3
        if (touch.x >= 0 && touch.x <= 80 && touch.y >= 0 && touch.y <= 50)
        {
          // go to page 1
          gotopage1();
        }
        // Right Arrow
        else if (touch.x >= 240 && touch.x <= 320 && touch.y >= 0 && touch.y <= 50)
        {
        }
        // Button 1 (WALLET)
        else if (touch.x >= 18 && touch.x <= 18 + 140 && touch.y >= 50 && touch.y <= 48 + 90)
        {
          pageIndex = 9;
          menuId = 6;
          showPage9();
          delay(500);
        }
        // Button 2 (SOCIAL)
        else if (touch.x >= 18 && touch.x <= 18 + 140 && touch.y >= 140 && touch.y <= 140 + 90)
        {
          pageIndex = 9;
          menuId = 7;
          showPage9();
          delay(500);
        }
        // Button 3 (Contact)
        else if (touch.x >= tft.width() - 18 - 140 && touch.x <= tft.width() - 18 && touch.y >= 50 && touch.y <= 48 + 90)
        {
          pageIndex = 9;
          menuId = 8;
          showPage9();
          delay(500);
        }
        // Button 4 (Badege)
        else if (touch.x >= tft.width() - 18 - 140 && touch.x <= tft.width() - 18 && touch.y >= 140 && touch.y <= 140 + 90)
        {
          // underconstruction
        }
      }
      else if (menuId == 6) // wallet
      {
        // Left Arrow to Profile Menu
        if (touch.x >= 0 && touch.x <= 80 && touch.y >= 0 && touch.y <= 50)
        {
          pageIndex = 9;
          menuId = 5;
          showPage9();
          delay(500);
        }
        // Right Arrow
        else if (touch.x >= 240 && touch.x <= 320 && touch.y >= 0 && touch.y <= 50)
        {
        }
        // Button 1 (PROMPTPAY )
        else if (touch.x >= 18 && touch.x <= 18 + 140 && touch.y >= 50 && touch.y <= 48 + 90)
        {
          // การกระทำเมื่อกดปุ่ม RECEIVE
          pageIndex = 17;
          currentWallet = 0; //{"PROMPTPAY", "USDT", "SAT", "CCT"}
          coinReceive = 1;   // 0 = shop , 1 = user
          amounts = 0.00;
          showPage17();
          delay(500);
        }
        // Button 2 (USDT)
        else if (touch.x >= 18 && touch.x <= 18 + 140 && touch.y >= 140 && touch.y <= 140 + 90)
        {
          // การกระทำเมื่อกดปุ่ม RECEIVE
          pageIndex = 16;
          currentWallet = 1;
          menuId = 6;
          showPage16();
          delay(500);
        }
        // Button 3 (SAT)
        else if (touch.x >= tft.width() - 18 - 140 && touch.x <= tft.width() - 18 && touch.y >= 50 && touch.y <= 48 + 90)
        {
          pageIndex = 16;
          currentWallet = 2;
          menuId = 6;
          showPage16();
          delay(500);
        }
        // Button 4 (CCT)
        else if (touch.x >= tft.width() - 18 - 140 && touch.x <= tft.width() - 18 && touch.y >= 140 && touch.y <= 140 + 90)
        {
          pageIndex = 16;
          currentWallet = 3;
          menuId = 6;
          showPage16();
          delay(500);
        }
      }
      else if (menuId == 7) // social
      {
        // Left Arrow to Profile Menu
        if (touch.x >= 0 && touch.x <= 80 && touch.y >= 0 && touch.y <= 50)
        {
          pageIndex = 9;
          menuId = 5;
          showPage9();
          delay(500);
        }
        // Right Arrow
        else if (touch.x >= 240 && touch.x <= 320 && touch.y >= 0 && touch.y <= 50)
        {
        }
        // Button 1 (WALLET)
        else if (touch.x >= 18 && touch.x <= 18 + 140 && touch.y >= 50 && touch.y <= 48 + 90)
        {
        }
        // Button 2 (SOCIAL)
        else if (touch.x >= 18 && touch.x <= 18 + 140 && touch.y >= 140 && touch.y <= 140 + 90)
        {
        }
        // Button 3 (Contact)
        else if (touch.x >= tft.width() - 18 - 140 && touch.x <= tft.width() - 18 && touch.y >= 50 && touch.y <= 48 + 90)
        {
        }
        // Button 4 (Badege)
        else if (touch.x >= tft.width() - 18 - 140 && touch.x <= tft.width() - 18 && touch.y >= 140 && touch.y <= 140 + 90)
        {
          // underconstruction
        }
      }
      else if (menuId == 8) // contact
      {
        // Left Arrow to Profile Menu
        if (touch.x >= 0 && touch.x <= 80 && touch.y >= 0 && touch.y <= 50)
        {
          pageIndex = 9;
          menuId = 5;
          showPage9();
          delay(500);
        }
        // Right Arrow
        else if (touch.x >= 240 && touch.x <= 320 && touch.y >= 0 && touch.y <= 50)
        {

        }
        // Button 1 (WALLET)
        else if (touch.x >= 18 && touch.x <= 18 + 140 && touch.y >= 50 && touch.y <= 48 + 90)
        {

        }
        // Button 2 (SOCIAL)
        else if (touch.x >= 18 && touch.x <= 18 + 140 && touch.y >= 140 && touch.y <= 140 + 90)
        {

        }
        // Button 3 (Contact)
        else if (touch.x >= tft.width() - 18 - 140 && touch.x <= tft.width() - 18 && touch.y >= 50 && touch.y <= 48 + 90)
        {

        }
        // Button 4 (Badege)
        else if (touch.x >= tft.width() - 18 - 140 && touch.x <= tft.width() - 18 && touch.y >= 140 && touch.y <= 140 + 90)
        {
          // underconstruction
        }
      }
      else if (menuId == 9) // 29 139 69 439
      {
        if (touch.x >= 0 && touch.x <= 80 && touch.y >= 0 && touch.y <= 50)
        {
          pageIndex = 24;
          showPage24();
          delay(500);
        }
        // Right Arrow
        else if (touch.x >= 240 && touch.x <= 320 && touch.y >= 0 && touch.y <= 50)
        {
          pageIndex = 9;
          menuId = 10;
          showPage9();
          delay(500);
        }
        // Button 1 (29)
        else if (touch.x >= 18 && touch.x <= 18 + 140 && touch.y >= 50 && touch.y <= 48 + 90)
        {
          selectedPrice = 29 ;
          pageIndex = 9;
          menuId = 4;
          showPage9();
          delay(500);
        }
        // Button 2 (69)
        else if (touch.x >= 18 && touch.x <= 18 + 140 && touch.y >= 140 && touch.y <= 140 + 90)
        {
          selectedPrice = 69 ;
          pageIndex = 9;
          menuId = 4;
          showPage9();
          delay(500);
        }
        // Button 3 (139)
        else if (touch.x >= tft.width() - 18 - 140 && touch.x <= tft.width() - 18 && touch.y >= 50 && touch.y <= 48 + 90)
        {
          selectedPrice = 139 ;
          pageIndex = 9;
          menuId = 4;
          showPage9();
          delay(500);
        }
        // Button 4 (439)
        else if (touch.x >= tft.width() - 18 - 140 && touch.x <= tft.width() - 18 && touch.y >= 140 && touch.y <= 140 + 90)
        {
          selectedPrice = 439 ;
          pageIndex = 9;
          menuId = 4;
          showPage9();
          delay(500);
        }
      }
      else if (menuId == 10) // 939 1639 2539 3639
      {
        if (touch.x >= 0 && touch.x <= 80 && touch.y >= 0 && touch.y <= 50)
        {

          pageIndex = 9;
          menuId = 9;
          showPage9();
          delay(500);
        }
        // Right Arrow
        else if (touch.x >= 240 && touch.x <= 320 && touch.y >= 0 && touch.y <= 50)
        {
          pageIndex = 9;
          menuId = 4;
          showPage9();
          delay(500);
        }
        // Button 1 (939)
        else if (touch.x >= 18 && touch.x <= 18 + 140 && touch.y >= 50 && touch.y <= 48 + 90)
        {
          selectedPrice = 939 ;
          pageIndex = 9;
          menuId = 4;
          showPage9();
          delay(500);
        }
        // Button 2 (1639)
        else if (touch.x >= 18 && touch.x <= 18 + 140 && touch.y >= 140 && touch.y <= 140 + 90)
        {
          selectedPrice = 1639 ;
          pageIndex = 9;
          menuId = 4;
          showPage9();
          delay(500);
        }
        // Button 3 (2539)
        else if (touch.x >= tft.width() - 18 - 140 && touch.x <= tft.width() - 18 && touch.y >= 50 && touch.y <= 48 + 90)
        {
          selectedPrice = 2539;
          pageIndex = 9;
          menuId = 4;
          showPage9();
          delay(500);
        }
        // Button 4 (3639)
        else if (touch.x >= tft.width() - 18 - 140 && touch.x <= tft.width() - 18 && touch.y >= 140 && touch.y <= 140 + 90)
        {
          selectedPrice = 3639;
          pageIndex = 9;
          menuId = 4;
          showPage9();
          delay(500);
        }
      }
    }
    else if (pageIndex == 10)
    {
    }
    else if (pageIndex == 11)
    { // Plan
      // Left Arrow
      if (touch.x >= 0 && touch.x <= 80 && touch.y >= 0 && touch.y <= 50)
      {
        // Code for Left Arrow  // Back to Page2
        pageIndex = 9;
        menuId = 0;
        showPage9();
        delay(500);
      }
    }
    else if (pageIndex == 12)
    { // Noti
      delay(200);
      if (!inputMode) // Only process if in input mode
      {
        // Left Arrow
        if (touch.x >= 0 && touch.x <= 80 && touch.y >= 50 && touch.y <= 100)
        {
          // Code for Left Arrow  // Back to Page2
          pageIndex = 9;
          menuId = 0;
          showPage9();
          delay(500);
        }
        // Switch Coin Button
        else if (touch.x >= 14 && touch.x <= 122 && touch.y >= 8 && touch.y <= 62)
        {
          // Code for BTC button
          toggleCoin(); // สลับเหรียญและอัปเดตการแสดงผล
        }
        // USDT Button
        else if (touch.x >= 200 && touch.x <= 308 && touch.y >= 8 && touch.y <= 62)
        {
          if (currencyAlert == currency1) {
            currencyAlert = currency2;
          } else {
            currencyAlert = currency1;
          }
          save_alert();
          showPage12();
        }
        // SET Button
        else if (touch.x >= 0 && touch.x <= 60 && touch.y >= 110 && touch.y <= 170)
        {
          if (coinIndex >= 0 && coinIndex < 4)
          { // Ensure coinIndex is within the valid range (0 to 3)
            int realIndex = ((currentAlertPage - 1) * 8) + alertSelectIndex; // Calculate the actual index based on the current page

            if (realIndex % 2 == 0)
            {
              // If realIndex is even, use Coin_Low
              if (coinLow[coinIndex][realIndex / 2] >= 10000.0)
              {
                inputText = String(coinLow[coinIndex][realIndex / 2], 1); // 1 decimal place
              }
              else if (coinLow[coinIndex][realIndex / 2] >= 1000.0)
              {
                inputText = String(coinLow[coinIndex][realIndex / 2], 2); // 2 decimal places
              }
              else if (coinLow[coinIndex][realIndex / 2] >= 100.0)
              {
                inputText = String(coinLow[coinIndex][realIndex / 2], 3); // 3 decimal places
              }
              else if (coinLow[coinIndex][realIndex / 2] >= 10.0)
              {
                inputText = String(coinLow[coinIndex][realIndex / 2], 4); // 4 decimal places
              }
              else if (coinLow[coinIndex][realIndex / 2] >= 1.0)
              {
                inputText = String(coinLow[coinIndex][realIndex / 2], 4); // Up to 4 decimal places
              }
              else
              {
                inputText = String(coinLow[coinIndex][realIndex / 2], 8); // 8 decimal places for values less than 1
              }
            }
            else
            {
              // If realIndex is odd, use Coin_High
              if (coinHigh[coinIndex][realIndex / 2] >= 10000.0)
              {
                inputText = String(coinHigh[coinIndex][realIndex / 2], 1); // 1 decimal place
              }
              else if (coinHigh[coinIndex][realIndex / 2] >= 1000.0)
              {
                inputText = String(coinHigh[coinIndex][realIndex / 2], 2); // 2 decimal places
              }
              else if (coinHigh[coinIndex][realIndex / 2] >= 100.0)
              {
                inputText = String(coinHigh[coinIndex][realIndex / 2], 3); // 3 decimal places
              }
              else if (coinHigh[coinIndex][realIndex / 2] >= 10.0)
              {
                inputText = String(coinHigh[coinIndex][realIndex / 2], 4); // 4 decimal places
              }
              else if (coinHigh[coinIndex][realIndex / 2] >= 1.0)
              {
                inputText = String(coinHigh[coinIndex][realIndex / 2], 4); // Up to 4 decimal places
              }
              else
              {
                inputText = String(coinHigh[coinIndex][realIndex / 2], 8); // 8 decimal places for values less than 1
              }
            }
          }
          else
          {
            // Handle invalid coinIndex if necessary
            inputText = "Invalid coin index";
          }

          // ลบเลข 0 ที่ไม่จำเป็นด้านหลัง
          while (inputText.endsWith("0") && inputText.indexOf('.') >= 0)
          {
            inputText.remove(inputText.length() - 1);
          }

          // ถ้าเหลือจุดทศนิยมอย่างเดียวให้ลบจุดทศนิยมออก
          if (inputText.endsWith("."))
          {
            inputText.remove(inputText.length() - 1);
          }

          // Switch to input mode
          inputMode = true;
          // Code for SET button
          drawKeypad();
        }

        // ON/OFF Button
        else if (touch.x >= 0 && touch.x <= 100 && touch.y >= 180 && touch.y <= 240)
        {
          // Code for ON/OFF button

          toggleStatus(alertSelectIndex);
        }
        // Up Arrow Button
        else if (touch.x >= 240 && touch.x <= 320 && touch.y >= 110 && touch.y <= 170)
        {
          // Code for Up Arrow button

          if (alertSelectIndex > 0)
          { // ตรวจสอบว่า index ยังไม่ต่ำกว่า 0
            alertSelectIndex--;
          }
          else
          {
            if (currentAlertPage > 1)
            {
              currentAlertPage--;   // เปลี่ยนหน้าเป็นหน้าก่อนหน้า
              alertSelectIndex = 7; // ตั้งค่าให้เลือกที่คู่สุดท้ายของหน้านี้
            }
            else
            {
              alertSelectIndex = 7; // ถ้าอยู่หน้า 1 ให้วนไปคู่สุดท้ายในหน้านั้น
            }
          }
          // เพิ่มการเรียกฟังก์ชันที่ใช้ในการแสดงผลถ้าจำเป็น เช่น showPage12();
          updateSelectedText(alertSelectIndex);
        }
        // Down Arrow Button
        else if (touch.x >= 240 && touch.x <= 320 && touch.y >= 175 && touch.y <= 235)
        {
          // Code for Down Arrow button
          if (alertSelectIndex < 7)
          { // ตรวจสอบว่า index ยังไม่เกิน 7
            alertSelectIndex++;
          }
          else
          {
            if (currentAlertPage < 2)
            {
              currentAlertPage++;   // เปลี่ยนหน้าเป็นหน้าถัดไป (คู่ที่ 5-8)
              alertSelectIndex = 0; // ตั้งค่าให้เลือกที่คู่แรกของหน้านี้
            }
            else
            {
              alertSelectIndex = 0; // ถ้าอยู่หน้า 2 ให้วนไปคู่แรกในหน้านั้น
            }
          }
          // เพิ่มการเรียกฟังก์ชันที่ใช้ในการแสดงผลถ้าจำเป็น เช่น showPage12();
          updateSelectedText(alertSelectIndex);
        }
      }
      else
      {

        // Check number keys 0-9
        // ตรวจจับการกดปุ่มสำหรับตัวเลข 0 และ 5
        if ((touch.x >= 5 && touch.x <= 60 && touch.y >= 66 && touch.y <= 121) || // ตำแหน่งของปุ่ม 0
            (touch.x >= 5 && touch.x <= 60 && touch.y >= 126 && touch.y <= 181))
        { // ตำแหน่งของปุ่ม 5
          if (inputText.indexOf('.') == -1 && inputText.length() < 9 || inputText.indexOf('.') != -1 && inputText.length() < 11)
          { // ตรวจสอบความยาว
            if (inputText == "0")
              inputText = "";
            inputText += (touch.y < 126) ? "0" : "5"; // ตรวจสอบว่ากดปุ่ม 0 หรือ 5
          }
          drawKeypad();
          return;
        }

        // ตรวจจับการกดปุ่มสำหรับตัวเลข 1 และ 6
        if ((touch.x >= 65 && touch.x <= 120 && touch.y >= 66 && touch.y <= 121) || // ตำแหน่งของปุ่ม 1
            (touch.x >= 65 && touch.x <= 120 && touch.y >= 126 && touch.y <= 181))
        { // ตำแหน่งของปุ่ม 6
          if (inputText.indexOf('.') == -1 && inputText.length() < 9 || inputText.indexOf('.') != -1 && inputText.length() < 11)
          { // ตรวจสอบความยาว
            if (inputText == "0")
              inputText = "";
            inputText += (touch.y < 126) ? "1" : "6"; // ตรวจสอบว่ากดปุ่ม 1 หรือ 6
          }
          drawKeypad();
          return;
        }

        // ตรวจจับการกดปุ่มสำหรับตัวเลข 2 และ 7
        if ((touch.x >= 125 && touch.x <= 175 && touch.y >= 66 && touch.y <= 121) || // ตำแหน่งของปุ่ม 2
            (touch.x >= 125 && touch.x <= 175 && touch.y >= 126 && touch.y <= 181))
        { // ตำแหน่งของปุ่ม 7
          if (inputText.indexOf('.') == -1 && inputText.length() < 9 || inputText.indexOf('.') != -1 && inputText.length() < 11)
          { // ตรวจสอบความยาว
            if (inputText == "0")
              inputText = "";
            inputText += (touch.y < 126) ? "2" : "7"; // ตรวจสอบว่ากดปุ่ม 2 หรือ 7
          }
          drawKeypad();
          return;
        }

        // ตรวจจับการกดปุ่มสำหรับตัวเลข 3 และ 8
        if ((touch.x >= 175 && touch.x <= 220 && touch.y >= 66 && touch.y <= 121) || // ตำแหน่งของปุ่ม 3
            (touch.x >= 175 && touch.x <= 220 && touch.y >= 126 && touch.y <= 181))
        { // ตำแหน่งของปุ่ม 8
          if (inputText.indexOf('.') == -1 && inputText.length() < 9 || inputText.indexOf('.') != -1 && inputText.length() < 11)
          { // ตรวจสอบความยาว
            if (inputText == "0")
              inputText = "";
            inputText += (touch.y < 126) ? "3" : "8"; // ตรวจสอบว่ากดปุ่ม 3 หรือ 8
          }
          drawKeypad();
          return;
        }

        // ตรวจจับการกดปุ่มสำหรับตัวเลข 4 และ 9
        if ((touch.x >= 220 && touch.x <= 320 && touch.y >= 66 && touch.y <= 121) || // ตำแหน่งของปุ่ม 4
            (touch.x >= 220 && touch.x <= 320 && touch.y >= 126 && touch.y <= 181))
        { // ตำแหน่งของปุ่ม 9
          if (inputText.indexOf('.') == -1 && inputText.length() < 9 || inputText.indexOf('.') != -1 && inputText.length() < 11)
          { // ตรวจสอบความยาว
            if (inputText == "0")
              inputText = "";
            inputText += (touch.y < 126) ? "4" : "9"; // ตรวจสอบว่ากดปุ่ม 4 หรือ 9
          }
          drawKeypad();
          return;
        }

        // Check . key
        int dotY = keyStartY + (12 / 5) * (keyHeight + keyPadding);
        if (touch.x >= 125 && touch.x <= 175 && touch.y >= dotY && touch.y <= dotY + keyHeight)
        {
          if (inputText.indexOf('.') == -1 && inputText.length() < 9 && inputText.toFloat() < 10000)
          { // Add dot if not already present and length allows
            inputText += ".";
          }
          drawKeypad(); // Redraw keypad to update text
          return;
        }

        // ตรวจจับการกดปุ่ม D ใต้ปุ่ม 6
        if (touch.x >= 65 && touch.x <= 120 && touch.y >= 186 && touch.y <= 241)
        { // ตำแหน่งของปุ่ม D
          if (inputText.length() > 1)
          {
            inputText.remove(inputText.length() - 1); // ลบตัวอักษรสุดท้าย
          }
          else
          {
            inputText = "0"; // ถ้าเหลือตัวเดียว ปรับให้เป็น 0
          }
          drawKeypad();
          return;
        }

        // Check C (Clear) key
        int clearX = keyStartX + (13 % 5) * (keyWidth + keyPadding);
        int clearY = keyStartY + (13 / 5) * (keyHeight + keyPadding);
        if (touch.x >= clearX && touch.x <= clearX + keyWidth && touch.y >= clearY && touch.y <= clearY + keyHeight)
        {
          inputText = "0"; // Clear input
          drawKeypad();    // Redraw keypad to update text
          return;
        }

        // Check OK key
        int okX = keyStartX + (14 % 5) * (keyWidth + keyPadding);
        int okY = keyStartY + (14 / 5) * (keyHeight + keyPadding);
        if (touch.x >= okX && touch.x <= okX + keyWidth && touch.y >= okY && touch.y <= okY + keyHeight)
        {
          // Confirm input, copy to selected alert text
          saveInputTextToAlert(alertSelectIndex, inputText);
          save_alert();
          inputMode = false; // Exit input mode
          showPage12();      // Redraw main page
          return;
        }

        // Check Left Arrow (Back) key
        if (touch.x >= 0 && touch.x <= 60 && touch.y >= 180 && touch.y <= 241)
        {
          // Exit input mode without saving changes
          inputMode = false;
          showPage12(); // Redraw main page with previous value
          return;
        }
      }
    }
    else if (pageIndex == 13)
    { // ota
      // Left Arrow
      if (touch.x >= 0 && touch.x <= 80 && touch.y >= 0 && touch.y <= 50)
      {
        // Code for Left Arrow  // Back to Page2
        pageIndex = 9;
        menuId = 2;
        showPage9();
        delay(500);
      }
    }
    else if (pageIndex == 14)
    {
      // Left Arrow
      if (touch.x >= 0 && touch.x <= 80 && touch.y >= 0 && touch.y <= 50)
      {
        // Code for Left Arrow  // Back to Settihg4
        pageIndex = 9;
        menuId = 3;
        showPage9();
        delay(500);
      }
    }
    else if (pageIndex == 15)
    {
      // Left Arrow
      if (touch.x >= 0 && touch.x <= 80 && touch.y >= 0 && touch.y <= 50)
      {
        // Code for Left Arrow  // Back to Page2
        pageIndex = 9;
        menuId = 0;
        showPage9();
        delay(500);
      }
    }
    else if (pageIndex == 16)
    {
      // Left Arrow
      if (touch.x >= 0 && touch.x <= 80 && touch.y >= 0 && touch.y <= 50)
      {
        // Code for Left Arrow  // Back to Page2
        pageIndex = 9;
        menuId = 6;
        showPage9();
        delay(500);
      }

      if (touch.x >= 10 && touch.x <= 120 && touch.y >= 175 && touch.y <= 240)
      {
        // การกระทำเมื่อกดปุ่ม RECEIVE
        Serial.println("RECEIVE button pressed");
        pageIndex = 17;
        if (currentWallet == 0) // pp
        {
          amounts = 0.00;
        }
        else if (currentWallet == 1) // usdt
        {
          amounts = 0.00;
        }
        else if (currentWallet == 2) // sat
        {
          coinOutput = 1.00;
          coinInput = coinOutput * rates[4]; // Update sat based on THB
          amounts = 1.00;
        }
        else if (currentWallet == 3) // cct
        {
          amounts = 0.00;
        }

        coinReceive = 1; // 0 = shop , 1 = user

        showPage17();
        delay(500);
      }

      if (touch.x >= 200 && touch.x <= 310 && touch.y >= 175 && touch.y <= 235)
      {
        // การกระทำเมื่อกดปุ่ม POS
        Serial.println("POS button pressed");
      }
    }
    else if (pageIndex == 17)
    {

      if (!inputMode)
      {
        // Left Arrow Back
        if (touch.x >= 0 && touch.x <= 80 && touch.y >= 0 && touch.y <= 50)
        {
          if (coinReceive == 0) // shop
          {
            menuId = 4;
          }
          else //customer
          {
            menuId = 6;
          }
          pageIndex = 9;

          showPage9();
          delay(500);
        }
        else if (coinReceive == 0 )
        {
          return;
        }

        // Check for left number (THB) click to display keypad
        else if ((touch.x >= 0 && touch.x <= 135) && (touch.y >= 180 && touch.y <= 240))
        {
          inputMode = true;
          editingInput = true; // Editing Input
          inputText = String(coinInput);
          drawKeypad();
        }

        // Check for right number (CCP) click to display keypad
        else if ((touch.x >= 185 && touch.x <= 320) && (touch.y >= 180 && touch.y <= 240))
        {
          inputMode = true;
          editingInput = false; // Editing Output
          inputText = String(coinOutput);
          drawKeypad();
        }
      }

      else
      {
        // Check number keys 0-9
        // ตรวจจับการกดปุ่มสำหรับตัวเลข 0 และ 5
        if ((touch.x >= 5 && touch.x <= 60 && touch.y >= 66 && touch.y <= 121) || // ตำแหน่งของปุ่ม 0
            (touch.x >= 5 && touch.x <= 60 && touch.y >= 126 && touch.y <= 181))
        { // ตำแหน่งของปุ่ม 5
          if (inputText.indexOf('.') == -1 && inputText.length() < 9 || inputText.indexOf('.') != -1 && inputText.length() < 11)
          { // ตรวจสอบความยาว
            if (inputText == "0")
              inputText = "";
            inputText += (touch.y < 126) ? "0" : "5"; // ตรวจสอบว่ากดปุ่ม 0 หรือ 5
          }
          drawKeypad();
          return;
        }

        // ตรวจจับการกดปุ่มสำหรับตัวเลข 1 และ 6
        if ((touch.x >= 65 && touch.x <= 120 && touch.y >= 66 && touch.y <= 121) || // ตำแหน่งของปุ่ม 1
            (touch.x >= 65 && touch.x <= 120 && touch.y >= 126 && touch.y <= 181))
        { // ตำแหน่งของปุ่ม 6
          if (inputText.indexOf('.') == -1 && inputText.length() < 9 || inputText.indexOf('.') != -1 && inputText.length() < 11)
          { // ตรวจสอบความยาว
            if (inputText == "0")
              inputText = "";
            inputText += (touch.y < 126) ? "1" : "6"; // ตรวจสอบว่ากดปุ่ม 1 หรือ 6
          }
          drawKeypad();
          return;
        }

        // ตรวจจับการกดปุ่มสำหรับตัวเลข 2 และ 7
        if ((touch.x >= 125 && touch.x <= 175 && touch.y >= 66 && touch.y <= 121) || // ตำแหน่งของปุ่ม 2
            (touch.x >= 125 && touch.x <= 175 && touch.y >= 126 && touch.y <= 181))
        { // ตำแหน่งของปุ่ม 7
          if (inputText.indexOf('.') == -1 && inputText.length() < 9 || inputText.indexOf('.') != -1 && inputText.length() < 11)
          { // ตรวจสอบความยาว
            if (inputText == "0")
              inputText = "";
            inputText += (touch.y < 126) ? "2" : "7"; // ตรวจสอบว่ากดปุ่ม 2 หรือ 7
          }
          drawKeypad();
          return;
        }

        // ตรวจจับการกดปุ่มสำหรับตัวเลข 3 และ 8
        if ((touch.x >= 175 && touch.x <= 220 && touch.y >= 66 && touch.y <= 121) || // ตำแหน่งของปุ่ม 3
            (touch.x >= 175 && touch.x <= 220 && touch.y >= 126 && touch.y <= 181))
        { // ตำแหน่งของปุ่ม 8
          if (inputText.indexOf('.') == -1 && inputText.length() < 9 || inputText.indexOf('.') != -1 && inputText.length() < 11)
          { // ตรวจสอบความยาว
            if (inputText == "0")
              inputText = "";
            inputText += (touch.y < 126) ? "3" : "8"; // ตรวจสอบว่ากดปุ่ม 3 หรือ 8
          }
          drawKeypad();
          return;
        }

        // ตรวจจับการกดปุ่มสำหรับตัวเลข 4 และ 9
        if ((touch.x >= 220 && touch.x <= 320 && touch.y >= 66 && touch.y <= 121) || // ตำแหน่งของปุ่ม 4
            (touch.x >= 220 && touch.x <= 320 && touch.y >= 126 && touch.y <= 181))
        { // ตำแหน่งของปุ่ม 9
          if (inputText.indexOf('.') == -1 && inputText.length() < 9 || inputText.indexOf('.') != -1 && inputText.length() < 11)
          { // ตรวจสอบความยาว
            if (inputText == "0")
              inputText = "";
            inputText += (touch.y < 126) ? "4" : "9"; // ตรวจสอบว่ากดปุ่ม 4 หรือ 9
          }
          drawKeypad();
          return;
        }

        // Check . key
        int dotY = keyStartY + (12 / 5) * (keyHeight + keyPadding);
        if (touch.x >= 125 && touch.x <= 175 && touch.y >= dotY && touch.y <= dotY + keyHeight)
        {
          if (inputText.indexOf('.') == -1 && inputText.length() < 9 && inputText.toFloat() < 10000)
          { // Add dot if not already present and length allows
            inputText += ".";
          }
          drawKeypad(); // Redraw keypad to update text
          return;
        }

        // ตรวจจับการกดปุ่ม D ใต้ปุ่ม 6
        if (touch.x >= 65 && touch.x <= 120 && touch.y >= 186 && touch.y <= 241)
        { // ตำแหน่งของปุ่ม D
          if (inputText.length() > 1)
          {
            inputText.remove(inputText.length() - 1); // ลบตัวอักษรสุดท้าย
          }
          else
          {
            inputText = "0"; // ถ้าเหลือตัวเดียว ปรับให้เป็น 0
          }
          drawKeypad();
          return;
        }

        // Check C (Clear) key
        int clearX = keyStartX + (13 % 5) * (keyWidth + keyPadding);
        int clearY = keyStartY + (13 / 5) * (keyHeight + keyPadding);
        if (touch.x >= clearX && touch.x <= clearX + keyWidth && touch.y >= clearY && touch.y <= clearY + keyHeight)
        {
          inputText = "0"; // Clear input
          drawKeypad();    // Redraw keypad to update text
          return;
        }

        // Check OK key
        int okX = keyStartX + (14 % 5) * (keyWidth + keyPadding);
        int okY = keyStartY + (14 / 5) * (keyHeight + keyPadding);
        if (touch.x >= okX && touch.x <= okX + keyWidth && touch.y >= okY && touch.y <= okY + keyHeight)
        {
          if (editingInput)
          {
            coinInput = inputText.toFloat();
            if (currentWallet == 0) // promptpay - > Sats
            {
              coinOutput = coinInput / rates[4]; // Update Sat based on THB
              amounts = ((float)((int)(coinInput * 100 + 0.5))) / 100;
            }
            else if (currentWallet == 1) // usdt
            {
              coinOutput = coinInput / rates[5]; // Update usdt based on THB
              amounts = ((float)((int)(coinOutput * 100 + 0.5))) / 100;
            }
            else if (currentWallet == 2) // sat
            {
              if (coinInput == 0)
              {
                coinInput = 1;
              }
              coinOutput = coinInput / rates[4]; // Update sat based on THB
              amounts = ((float)((int)(coinOutput * 100 + 0.5))) / 100;
            }
            else if (currentWallet == 3) // ccp
            {
              coinOutput = coinInput / rates[6]; // Update CCP based on THB
              amounts = ((float)((int)(coinOutput * 100 + 0.5))) / 100;
            }
          }
          else
          {
            coinOutput = inputText.toFloat();
            if (currentWallet == 0) // promptpay - > Sats
            {
              coinInput = coinOutput * rates[4]; // Update Sat based on THB
              amounts = ((float)((int)(coinInput * 100 + 0.5))) / 100;
            }
            else if (currentWallet == 1) // usdt
            {
              coinInput = coinOutput * rates[5]; // Update usdt based on THB
              amounts = ((float)((int)(coinOutput * 100 + 0.5))) / 100;
            }
            else if (currentWallet == 2) // sat
            {
              if (coinOutput == 0)
              {
                coinOutput = 1;
              }
              coinInput = coinOutput * rates[4]; // Update sat based on THB
              amounts = ((float)((int)(coinOutput * 100 + 0.5))) / 100;
            }
            else if (currentWallet == 3) // ccp
            {
              coinInput = coinOutput * rates[6]; // Update CCP based on THB
              amounts = ((float)((int)(coinOutput * 100 + 0.5))) / 100;
            }
          }
          // update qr
          inputMode = false; // Exit input mode

          pageIndex = 17;
          showPage17(); // Redraw main page

          return;
        }

        // Check Left Arrow (Back) key
        if (touch.x >= 0 && touch.x <= 60 && touch.y >= 180 && touch.y <= 241)
        {
          // Exit input mode without saving changes
          inputMode = false;
          pageIndex = 17;
          showPage17(); // Redraw main page with previous value
          return;
        }
      }
    }
    else if (pageIndex == 19)
    { // blank
      // Left Arrow
      if (touch.x >= 0 && touch.x <= 80 && touch.y >= 0 && touch.y <= 50)
      {
        // Code for Left Arrow  // Back to Page2
        pageIndex = 9;
        menuId = 0;
        showPage9();
        delay(100);
      }
    }
    else if (pageIndex == 20)
    { // blank
    }
    else if (pageIndex == 21)
    { // blank
    }
    else if (pageIndex == 22)
    { // Notification
      // Snooze
      if (touch.x >= 190 && touch.x <= 320 && touch.y >= 170 && touch.y <= 240)
      {
        alertTime = millis();
        pauseAlert = true;
        size_t heapSize = ESP.getFreeHeap();
        Serial.println(heapSize);
        gotopage2();
      }

      if (touch.x >= 0 && touch.x <= 120 && touch.y >= 170 && touch.y <= 240)
      {
        String a = "I:" + String(alertI) + " J:" + String(alertJ);
        Serial.println(a);
        if (alertStatus == "LOW")
        {
          coinLowStatus[alertI][alertJ] = false;
        }
        else if (alertStatus == "HIGH")
        {
          coinHighStatus[alertI][alertJ] = false;
        }

        save_alert();
        size_t heapSize = ESP.getFreeHeap();
        Serial.println(heapSize);
        gotopage2();
      }
    }
    else if (pageIndex == 23)
    { // ComingSoon

      // Left Arrow
      if (touch.x >= 0 && touch.x <= 80 && touch.y >= 0 && touch.y <= 50)
      {
        // Code for Left Arrow  // Back to Page2
        pageIndex = 9;
        menuId = 0;
        showPage9();
        delay(500);
      }
    }
    else if (pageIndex == 24) // token page
    { // Token
      // Left Arrow
      if (touch.x >= 0 && touch.x <= 80 && touch.y >= 0 && touch.y <= 50)
      {
        // Code for Left Arrow  // Back to Page2
        pageIndex = 9;
        menuId = 1;
        showPage9();
        delay(500);
      }
      // TOP UP - > Select Money
      if (touch.x >= 105 && touch.x <= 220 && touch.y >= 175 && touch.y <= 240)
      {
        // Code for Left Arrow  // Back to Page2
        pageIndex = 9;
        menuId = 9;
        Serial.print("→ [DEBUG] Before showPage9(), menuId = ");
        Serial.println(menuId);
        showPage9();
        delay(50);
      }
    }
    else if (pageIndex == 25)
    {
      // Left button: Normal Black
      if (touch.x >= 30 && touch.x <= 140 && touch.y >= 150 && touch.y <= 200)
      {
        inversion = true;
        saveInversion(inversion);
        delay(500);
        ESP.restart(); // Restart after saving
      }
      // Right button: Normal White
      else if (touch.x >= 170 && touch.x <= 280 && touch.y >= 150 && touch.y <= 200)
      {
        inversion = false;
        saveInversion(inversion);
        delay(500);
        ESP.restart(); // Restart after saving
      }

    }
  }
}
