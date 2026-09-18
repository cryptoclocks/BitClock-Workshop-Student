#ifndef PAGE1_H
#define PAGE1_H
#include <Arduino.h>
#include "GlobalModule.h"
#include "GlobalParameter.h"

#include "page2.h"
#include "page6.h"
#include "GlobalFunction.h"
#include <ArduinoJson.h>

#include <FS.h>
#include "JpgSupport.h"

fs::File file2;

// Forward declarations
int cl ;

void updateTime();
void showPage1();

// ฟังก์ชันเพื่อแปลงสตริงสีเป็น unsigned int
// ฟังก์ชันเพิ่มเติมสำหรับแปลงสี HEX เป็น RGB
uint32_t convertColor(String hex) {
  hex.trim();
  if (hex.startsWith("#")) hex = hex.substring(1);
  long number = strtol(&hex[0], NULL, 16);
  return tft.color565(number >> 16, (number >> 8) & 0xFF, number & 0xFF);
}

void customDrawMaskedImage(int centerX, int centerY, int radius , int color) {
  for (int x = centerX - radius; x <= centerX + radius; x++) {
    for (int y = centerY - radius; y <= centerY + radius; y++) {
      if ((x - centerX) * (x - centerX) + (y - centerY) * (y - centerY) <= radius * radius - 100) {

      }
      else
      {
        tft.drawPixel(x, y, color);
      }
    }
  }
}

// Global variables to store old times
char lastTime[10] = ""; // Stores the last time displayed
char lastDate[20] = ""; // Stores the last date displayed

void updateTime() {
  Serial.println("Update Time");
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Failed to obtain time");
    return;
  }

  char timeString[10];
  char timeString2[20];
  strftime(timeString, sizeof(timeString), "%H:%M", &timeinfo);
  strftime(timeString2, sizeof(timeString2), "%d/%m/%Y", &timeinfo);

  // Check if the minute or hour has changed
  if (strcmp(lastTime, timeString) != 0 || pageUpdate == true) {
    // Clear the area where the old time was displayed
    tft.fillRect(152, 60, 160, 40, convertColor(profile_bgcolor)); // Adjust size as necessary

    // Convert the datetime color from HEX to TFT color format
    uint32_t color = convertColor(profile_datetimecolor);
    Serial.println(timeString);
    // Display the new time
    tft.setCursor(152, 60);
    tft.setTextColor(color); // Use the color specified for date/time
    tft.setTextSize(5); // Set text size for time
    tft.print(timeString);

    strcpy(lastTime, timeString); // Update lastTime with the new time
  }

  // Check if the date has changed
  if (strcmp(lastDate, timeString2) != 0  || pageUpdate == true) {
    pageUpdate = false ;
    // Clear the area where the old date was displayed
    tft.fillRect(165, 110, 150, 20, convertColor(profile_bgcolor)); // Adjust size as necessary

    // Display the new date
    tft.setCursor(165, 110);
    tft.setTextSize(2); // Set text size for date
    tft.print(timeString2);

    strcpy(lastDate, timeString2); // Update lastDate with the new date
  }

  if (lotteryAlert == true)
  {
    // Check for lottery days and 5 PM
    int day1 = timeinfo.tm_mday;
    int hour1 = timeinfo.tm_hour;
    int minute1 = timeinfo.tm_min;
    //    Serial.println("Check Lottery");
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
  }
}


void showPage1() {
  tft.setTextDatum(TL_DATUM);

  String imagePathStr = "/" + firmwareVersion + "/system/image/profile.jpg";
  const char* imagePath = imagePathStr.c_str();
  // ใช้ตัวแปรที่โหลดไว้จาก load_profile() ไม่ต้องอ่านไฟล์ใหม่
  tft.fillScreen(convertColor(profile_bgcolor));

  // แสดงภาพโปรไฟล์
  drawSdJpeg(imagePath, 10, 10);
  customDrawMaskedImage(60, 60, 50, convertColor(profile_bgcolor));

  // แสดงชื่อนามสกุล
  String nameSurname = profile_firstname + " " + profile_lastname;
  String positionNickname = "(" + profile_nickname + ") " + profile_position;

  tft.setTextColor(convertColor(profile_namecolor));
  tft.setTextSize(2);
  tft.setCursor(10, 140);
  tft.print(nameSurname);

  // แสดงตำแหน่งและชื่อบริษัท
  tft.setTextColor(convertColor(profile_detailcolor));
  tft.setTextSize(2);
  tft.setCursor(10, 175);
  tft.print(positionNickname);

  tft.setCursor(10, 205);
  tft.print(profile_company);

  // แสดงคำคม
  String motto = profile_motto;
  tft.setTextColor(convertColor(profile_mottocolor));
  tft.setTextSize(1);

  int startX = 107;
  int endX = 315;
  int rangeWidth = endX - startX;
  if (motto == "Don't Trust, Verify") {
    motto = "Don't Trust,Verify";
  }

  // กำหนดขนาดฟอนต์ตามความยาวข้อความ
  int fontSize;
  if (motto.length() <= 13) {
    fontSize = 3;
  } else if (motto.length() <= 20) {
    fontSize = 2;
  } else {
    fontSize = 1;
  }

  int charWidth = 6 * fontSize;
  int textWidth1 = charWidth * motto.length();
  int center_x = startX + (rangeWidth - textWidth1) / 2;

  tft.setTextSize(fontSize);
  tft.setCursor(center_x, 11);
  tft.drawString(motto, center_x, 15);

  // อัปเดตและแสดงเวลาปัจจุบัน
  tft.setTextSize(1);
  tft.setTextColor(TFT_WHITE);
  updateTime();

  tft.setTextColor(TFT_WHITE);
}



#endif
