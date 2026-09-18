#include <Arduino.h>
#include "GlobalModule.h"
#include "GlobalParameter.h"
#include "GlobalFunction.h"
#include "time.h"
#include <HTTPClient.h>
#include <ArduinoJson.h>

#include <FS.h>

#include "JpgSupport.h"
#define MAX_IMAGE_WIDTH 320


// Variables to store the results
String date;
char prizeFirst[7];
String runningNumberFrontThree_1 = "000";
String runningNumberBackThree_1 = "000";
String runningNumberFrontThree_2 = "000";
String runningNumberBackThree_2 = "000";
String runningNumberBackTwo = "00";

void updateTime6() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Failed to obtain time");
    return;
  }

  char timeString[20];
  strftime(timeString, sizeof(timeString), "%H:%M:%S %d/%m/%Y", &timeinfo);
  Serial.println(timeString); // Print the time to the Serial Monitor

  // Display the time on the TFT
  tft.setCursor(100, 70);
  tft.setTextColor(TFT_WHITE); // Set text color
  tft.setTextSize(1); // Set text size
  tft.print(timeString);
}

String extractValue(String payload, String key) {
  int startIndex = payload.indexOf("\"" + key + "\":[\"") + key.length() + 4;
  int endIndex = payload.indexOf("\"", startIndex);
  if (startIndex > key.length() + 3 && endIndex > startIndex) {
    return payload.substring(startIndex, endIndex);
  } else {
    return "N/A";
  }
}


void fetchLotteryData(void *parameter) {
  const int maxRetries = 3;
  bool success = false;

  for (int attempt = 0; attempt < maxRetries; attempt++) {

    http3.begin("https://lotto.api.rayriffy.com/latest");
    int httpResponseCode = http3.GET();

    if (httpResponseCode > 0) {
      String payload = http3.getString();
      Serial.println("Payload:");
      Serial.println(payload);

       // รางวัลที่ 1
      int firstPrizeStart = payload.indexOf("\"number\":[\"") + 11;
      int firstPrizeEnd = payload.indexOf("\"", firstPrizeStart);
      String prizeFirstString = payload.substring(firstPrizeStart, firstPrizeEnd);
      prizeFirstString.toCharArray(prizeFirst, 7);

      // เลขหน้าสามตัว
      int frontThreeStart = payload.indexOf("\"runningNumberFrontThree") + 34;
      int frontThree1Start = payload.indexOf("[\"", frontThreeStart) + 2;
      int frontThree1End = payload.indexOf("\"", frontThree1Start);
      runningNumberFrontThree_1 = payload.substring(frontThree1Start, frontThree1End);

      int frontThree2Start = payload.indexOf("\",\"", frontThree1End) + 3;
      int frontThree2End = payload.indexOf("\"", frontThree2Start);
      runningNumberFrontThree_2 = payload.substring(frontThree2Start, frontThree2End);

      // เลขท้ายสามตัว
      int backThreeStart = payload.indexOf("\"runningNumberBackThree") + 34;
      int backThree1Start = payload.indexOf("[\"", backThreeStart) + 2;
      int backThree1End = payload.indexOf("\"", backThree1Start);
      runningNumberBackThree_1 = payload.substring(backThree1Start, backThree1End);

      int backThree2Start = payload.indexOf("\",\"", backThree1End) + 3;
      int backThree2End = payload.indexOf("\"", backThree2Start);
      runningNumberBackThree_2 = payload.substring(backThree2Start, backThree2End);

      // เลขท้ายสองตัว
      int backTwoStart = payload.indexOf("\"runningNumberBackTwo") + 34;
      int backTwoNumberStart = payload.indexOf("[\"", backTwoStart) + 2;
      int backTwoNumberEnd = payload.indexOf("\"", backTwoNumberStart);
      runningNumberBackTwo = payload.substring(backTwoNumberStart, backTwoNumberEnd);


      success = true;
      break;
    } else {
      Serial.println("Error on HTTP request");
    }

    http3.end();
    delay(2000);
  }

  if (success) {
    // Print to Serial
    Serial.print("Prize First: ");
    for (int i = 0; i < 6; i++) {
      Serial.print(prizeFirst[i]);
    }
    Serial.println();
    Serial.println("Running Number Front Three: " + runningNumberFrontThree_1 + "," + runningNumberFrontThree_2);
    Serial.println("Running Number Back Three: " + runningNumberBackThree_1  + "," + runningNumberBackThree_2);
    Serial.println("Running Number Back Two: " + runningNumberBackTwo);

    // Display on TFT
    tft.setTextColor(TFT_RED);
    tft.setTextSize(3);
    for (int i = 0; i < 6; i++) {
      tft.setCursor(29 + 50 * i, 95);
      tft.print(prizeFirst[i]);
    }

    tft.setTextSize(2);
    tft.setCursor(30, 158);
    tft.print(runningNumberFrontThree_1);

    tft.setTextSize(2);
    tft.setCursor(109, 158);
    tft.print(runningNumberFrontThree_2);

    tft.setTextSize(2);
    tft.setCursor(30, 210);
    tft.print(runningNumberBackThree_1);

    tft.setTextSize(2);
    tft.setCursor(109, 210);
    tft.print(runningNumberBackThree_2);

    tft.setTextSize(6);
    tft.setCursor(200, 170);
    tft.print(runningNumberBackTwo);
    tft.setTextColor(TFT_WHITE);

    updateTime6();
  } else {
    // Restart if data fetching failed
    ESP.restart();
  }

  vTaskDelete(NULL);  // Delete the task once done
}





void showPage6() {
  String imagePath = "/" + firmwareVersion +"/system/image/huaybg2.jpg"; // ชื่อไฟล์ภาพพื้นหลังของคุณ
  drawSdJpeg(imagePath.c_str(), 0, 0);

  //fetchdata
  xTaskCreate(fetchLotteryData, "FetchLotteryData", 8192, NULL, 1, NULL);

  //show
}
