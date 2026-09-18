#include <TFT_eSPI.h>   // Include the graphics library (this includes the sprite functions)
#include <SPI.h>
#include <SD.h>
#include <Arduino.h>
#include "GlobalModule.h"
#include "GlobalParameter.h"
#include "GlobalFunction.h"

#include "Page25.h"


String inversionFile = "/inversion.json";

void saveInversion(bool inv) {
  File file = LittleFS.open(inversionFile, "w");
  if (file) {
    StaticJsonDocument<64> doc;
    doc["inversion"] = inv;
    serializeJson(doc, file);
    file.close();
  }
}

bool loadInversion(bool &inv) {
  if (!LittleFS.exists(inversionFile)) {
    return false;
  }
  File file = LittleFS.open(inversionFile, "r");
  if (!file) {
    return false;
  }
  StaticJsonDocument<64> doc;
  DeserializationError error = deserializeJson(doc, file);
  file.close();
  if (error) {
    return false;
  }
  // เช็คว่าคีย์ inversion มีจริง
  if (doc.containsKey("inversion")) {
    inv = doc["inversion"];
    return true;
  }
  return false;
}

void drawInitialInversionPage(TFT_eSPI &tft) {
  // Show black background
    tft.setTextSize(2);
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextDatum(TL_DATUM);
  tft.drawString("Setup inversion", 10, 40, 2);

  tft.drawString("What Screen Color?", 10, 80, 2);

  // "Normal: Black" button (left)
  tft.fillRoundRect(30, 150, 110, 50, 8, TFT_DARKGREY);
  tft.setTextColor(TFT_WHITE, TFT_DARKGREY);
  tft.drawString("Black", 57, 160, 2);

  // "Normal: White" button (right)
  tft.fillRoundRect(170, 150, 110, 50, 8, TFT_WHITE);
  tft.setTextColor(TFT_BLACK, TFT_WHITE);
  tft.drawString("White", 192, 160, 2);
}


void checkInversion(TFT_eSPI &tft) {
  if (!LittleFS.begin()) {
    tft.fillScreen(TFT_RED);
    tft.setTextColor(TFT_WHITE, TFT_RED);
    tft.drawString("LittleFS not ready!", 10, 10, 2);
    delay(2000);
    return;
  }

  // ถ้าโหลดได้และมีค่าในไฟล์ ให้แทนที่และจบ
  if (loadInversion(inversion)) {
    // inversion ถูกกำหนดแล้ว
    Serial.println("inversion ถูกกำหนดแล้ว");
    Serial.println(inversion);
    return;
  }

  pageIndex = 25 ;
  // ถ้าไม่มีไฟล์หรือโหลดไม่ได้ ให้ถามผู้ใช้
  inversion = true; // ค่าเริ่มต้นพื้นหลังดำ
  drawInitialInversionPage(tft);

}
