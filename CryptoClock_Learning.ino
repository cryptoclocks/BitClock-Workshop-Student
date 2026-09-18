#include <Arduino.h>
#include <XPT2046_Bitbang.h>
#include <FS.h>
#include <SD.h>
#include <SPI.h>
#include <TFT_eSPI.h>
#include <WebServer.h>
#include "LittleFS.h"
#include "cc_littlefs.h"
#include "time.h"
#include <HTTPClient.h>
#include "GlobalModule.h"
#include "GlobalParameter.h"
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
#include "handle.h"

#include "GlobalFunction.h"
#include "JpgSupport.h"
#include "JPEGDecoder.h"
#include <ArduinoJson.h>
#include <esp_task_wdt.h>
#include "qrcode_gen.h"
#include "cc_aes.h"
#include <time.h>
#include "gif_draw.h"
#include <WiFiClientSecure.h>
#include "cc_sd.h"

//define
static unsigned long page4StartTime = 0; // To track when we entered page 4
const unsigned long page4Timeout = 5 * 60 * 1000; // 5 minutes timeout
static unsigned long page5StartTime = 0; // To track when we entered page 5
const unsigned long page5Timeout = 5 * 60 * 1000; // 5 minutes timeout
static unsigned long pageWiFiStartTime = 0; // To track when we entered page 5
const unsigned long pageWiFiTimeout = 3 * 60 * 1000; // 5 minutes timeout


#define MAX_IMAGE_WIDTH 320 // Adjust for your images

//touch
#define XPT2046_IRQ 36
#define XPT2046_MOSI 32
#define XPT2046_MISO 39
#define XPT2046_CLK 25
#define XPT2046_CS 33


TFT_eSPI tft = TFT_eSPI();         // Invoke custom library
XPT2046_Bitbang ts = XPT2046_Bitbang(XPT2046_MOSI, XPT2046_MISO, XPT2046_CLK, XPT2046_CS);


//local Parameter
String newurl2;
String newurl;
WebServer server(80);
String jsonWiFi ;
volatile bool permission = true;
volatile bool checkComplete = false;
bool isOn = false;

//slide
int currentIndex = 0;
uint16_t fileCount = 0;
uint32_t timer;
int pageSequenceIndex = 0;
//token
const int buttonPin = 0; // GPIO0

//slot
#define IMG_SIZE 80
int yPos = (240 - IMG_SIZE) / 2; // ตำแหน่ง Y เริ่มต้นตรงกลางจอ
int ySpeed = -1; // ความเร็วในการเลื่อนภาพ (ค่าลบคือเลื่อนขึ้น)
unsigned long lasPaymentInterval = 0 ;
File uploadFile;
const int chipSelect = 5;
unsigned long lastRateUpdateTime = 0;
const unsigned long rateUpdateInterval = 10 * 60 * 1000;





void  downloadAllCoinIcons(bool chkNew)
{
  const String baseCrypto   = "https://raw.githubusercontent.com/cryptoclocks/2025/main/assets/images/jpg48_crypto/";
  const String baseStock = "https://raw.githubusercontent.com/cryptoclocks/2025/main/assets/images/jpg48_stock/";
  const String baseCurrency = "https://raw.githubusercontent.com/cryptoclocks/2025/main/assets/images/jpg48_currency/";
  const String baseIndice   = "https://raw.githubusercontent.com/cryptoclocks/2025/main/assets/images/jpg48_indice/";

  for (int i = 0; i < 4; i++) {
    String local = "/" + firmwareVersion + "/system/image/coin" + String(i + 1) + ".jpg";
    if (SD.exists(local) && chkNew) {
      Serial.printf("✓ พบไอคอนแล้ว: %s\n", local.c_str());
    } else {
      String lowerCoin = coin[i];      lowerCoin.toLowerCase();
      String lowerMkt  = market[i];    lowerMkt.toLowerCase();

      String imageUrl;
      if (type[i] == "4") {                     // ✔️ หุ้นใช้ type = 4 เท่านั้น
        imageUrl = baseStock + lowerMkt + "/" + lowerCoin + ".jpg";
      } else if (type[i] == "0" || type[i] == "1" ||
                 type[i] == "2" || type[i] == "3") {
        imageUrl = baseCrypto   + lowerCoin + ".jpg";   // เหรียญ
      } else if (type[i] == "6") {
        imageUrl = baseCurrency + lowerCoin + ".jpg";   // สกุลเงิน
      } else if (type[i] == "7") {
        imageUrl = baseIndice + "indice.jpg";   // ดัชนี
      }

      bool status = downloadImage("coin" + String(i + 1), imageUrl.c_str());
      Serial.printf("  ↳ %s → %s\n", imageUrl.c_str(), status ? "OK" : "FAILED");
    }

  }
}


void initSDCard() {
  while (!SD.begin(5)) {
    Serial.println(F("🔁 SD.begin failed! Retrying in 1 sec..."));
    sdStatus = false;
    delay(1000); // รอ 1 วินาทีก่อนลองใหม่
  }
  Serial.println(F("✅ SD.begin OK!"));
  sdStatus = true;
}


void loadImage(int targetIndex) {
  String fileName = formatFileName(targetIndex + 1); // แก้ไขให้ใช้ชื่อไฟล์จาก SD card
  Serial.print("Loading: "); Serial.println(fileName);
  analogWrite(21, 0);
  drawSdJpeg(fileName.c_str(), 0, 0);
  if (isLowPowerMode) {
    for (int i = 0 ; i < 255 ; i++ )
    {
      TouchPoint touch = checkTouchScreen();
      if ((touch.zRaw != 0 )) {
        analogWrite(21, 255);
        return;
      }
      analogWrite(21, i);
      delay(10);
    }
  }
  if (isLowPowerMode) {
    for (int i = 0; i < 300; i++) {
      TouchPoint touch = checkTouchScreen();
      if ((touch.zRaw != 0)) {
        analogWrite(21, 255);
        return;
      }
      delay(10);
    }
  }
  if (isLowPowerMode) {
    for (int i = 0 ; i < 255 ; i++ )
    {
      TouchPoint touch = checkTouchScreen();
      if ((touch.zRaw != 0 )) {
        analogWrite(21, 255);
        return;
      }
      analogWrite(21, 255 - i);
      delay(10);
    }
    analogWrite(21, 0);
  }
  if (!isLowPowerMode) {
    {
      analogWrite(21, 255);
      return;
    }
  }
}

//end slide

float deg2rad(int degree) {
  return degree * PI / 180.0;
}

uint32_t hueToRGB(float hue) {
  int h = int(hue * 6);
  float f = hue * 6 - h;
  int p = 255 * (1 - 1);
  int q = 255 * (1 - f * 1);
  int t = 255 * (1 - (1 - f) * 1);

  switch (h % 6) {
    case 0: return tft.color565(255, t, p);
    case 1: return tft.color565(q, 255, p);
    case 2: return tft.color565(p, 255, t);
    case 3: return tft.color565(p, q, 255);
    case 4: return tft.color565(t, p, 255);
    case 5: return tft.color565(255, p, q);
  }
  return tft.color565(255, 255, 255); // Default to white
}

void drawLoadingAnimation(int &count, int totalIterations) {
  static int angle = 0;
  int radius = 50;
  int x_center = 160;
  int y_center = 120;
  int x_end, y_end;

  // Calculate hue based on current count vs total iterations
  float hue = (float)count / totalIterations;

  // Convert hue to RGB color
  // uint32_t color = hueToRGB(hue); // ไม่ใช้แล้ว เนื่องจากต้องการสีรุ้ง

  // Clear the drawing area by filling a black circle
  // tft.fillCircle(x_center, y_center, radius, TFT_BLACK);

  // Calculate angle for partial circle fill
  int fillAngle = map(count, 0, totalIterations, 0, 360);

  if (fillAngle % 10 != 0)
  {
    fillAngle = fillAngle + 10 - (fillAngle % 10 ) ;
  }

  // Draw pie slices with rainbow colors
  for (int i = 0; i <= fillAngle ; i += 10) {  // Increment by 10 degrees for each slice
    float rad = deg2rad(i);
    float rad_next = deg2rad(i + 10);
    int x1 = x_center + radius * cos(rad);
    int y1 = y_center + radius * sin(rad);
    int x2 = x_center + radius * cos(rad_next);
    int y2 = y_center + radius * sin(rad_next);

    // Calculate hue based on the angle for rainbow effect
    float sliceHue = (float)i / 360.0;
    uint32_t color = hueToRGB(sliceHue);

    // Draw filled triangle from center to these two points
    tft.fillTriangle(x_center, y_center, x1, y1, x2, y2, color);
  }

  // Increment the count for each frame
  count++;
}

void animateAndCount(int durationSeconds) {
  int count = 0;
  int delayTime = 100;  // Delay between frames in milliseconds
  int totalIterations = (durationSeconds * 1000) / delayTime;  // Calculate total iterations needed

  for (int i = 0; i < totalIterations; i++) {
    drawLoadingAnimation(count, totalIterations);  // Update animation and increment count
    delay(delayTime);
  }
  delay(500);
  // After the animation, print the count
  tft.fillScreen(TFT_BLACK);  // Clear the screen
}



void handleIndex1() {
  // ตรวจสอบว่าไฟล์ index.html มีอยู่ใน SD Card หรือไม่
  if (SD.exists("/" + firmwareVersion + "/system/website/index.html")) {
    File file = SD.open("/" + firmwareVersion + "/system/website/index.html", FILE_READ); // เปิดไฟล์สำหรับอ่าน
    if (file) {
      server.streamFile(file, "text/html"); // ส่งไฟล์ไปยังเซิร์ฟเวอร์
      file.close(); // ปิดไฟล์หลังใช้งาน
    } else {
      server.send(500, "text/html", "Failed to open index.html"); // กรณีเปิดไฟล์ไม่สำเร็จ
    }
  } else {
    server.send(404, "text/html", "index.html not found"); // กรณีไม่พบไฟล์
  }
}


void handleGallery() {
  // ตรวจสอบว่าไฟล์ index.html มีอยู่ใน SD Card หรือไม่
  if (SD.exists("/" + firmwareVersion + "/system/website/gallery.html")) {
    File file = SD.open("/" + firmwareVersion + "/system/website/gallery.html", FILE_READ); // เปิดไฟล์สำหรับอ่าน
    if (file) {
      server.streamFile(file, "text/html"); // ส่งไฟล์ไปยังเซิร์ฟเวอร์
      file.close(); // ปิดไฟล์หลังใช้งาน
    } else {
      server.send(500, "text/html", "Failed to open index.html"); // กรณีเปิดไฟล์ไม่สำเร็จ
    }
  } else {
    server.send(404, "text/html", "index.html not found"); // กรณีไม่พบไฟล์
  }
}

File fsUploadFile; // Global

void handleSettingsUpload() {
  HTTPUpload& upload = server.upload();
  if (upload.status == UPLOAD_FILE_START) {
    Serial.println("Start receiving settings.html...");
    File file = SD.open("/" + firmwareVersion + "/system/website/settings.html", FILE_WRITE);
    if (!file) {
      Serial.println("Failed to open settings.html for writing");
      return;
    }
    file.close();
  }
  else if (upload.status == UPLOAD_FILE_WRITE) {
    File file = SD.open("/" + firmwareVersion + "/system/website/settings.html", FILE_APPEND);
    if (file) {
      file.write(upload.buf, upload.currentSize);
      file.close();
    }
  }
  else if (upload.status == UPLOAD_FILE_END) {
    Serial.println("Upload of settings.html complete");
  }
}


void handleFileUpload() {
  HTTPUpload& upload = server.upload();
  if (upload.status == UPLOAD_FILE_START) {
    String filename = upload.filename;
    if (!filename.equals("")) {
      if (upload.name == "profile") { // Check if the file input name is "profile"
        filename = "/" + firmwareVersion + "/system/image/profile.jpg";
        File file = SD.open(filename, FILE_WRITE);
        if (!file) {
          Serial.println("File open failed");
          return;
        }
        file.close();
      }
    }
  } else if (upload.status == UPLOAD_FILE_WRITE) {
    if (upload.name == "profile") { // Check if the file input name is "profile"
      File file = SD.open("/" + firmwareVersion + "/system/image/profile.jpg", FILE_APPEND);
      if (file) {
        file.write(upload.buf, upload.currentSize);
        file.close();
      }
    }
  } else if (upload.status == UPLOAD_FILE_END) {
    if (upload.name == "profile") {
      Serial.println("Upload of profile.jpg complete");
      server.send(200, "text/html", "<!DOCTYPE html><html><head><meta http-equiv='refresh' content='10;url=/'></head><body>Settings saved. Restarting...</body></html>");
      delay(500);
    }
  }
}


void handleProfilePicture() {
  // ตรวจสอบว่าไฟล์ profile.jpg มีอยู่ใน SD Card หรือไม่
  if (SD.exists("/" + firmwareVersion + "/system/image/profile.jpg")) {
    File file = SD.open("/" + firmwareVersion + "/system/image/profile.jpg", FILE_READ); // เปิดไฟล์ในโหมดอ่าน
    if (file) {
      server.streamFile(file, "image/jpeg"); // ส่งไฟล์ไปยังไคลเอนต์
      file.close(); // ปิดไฟล์หลังการใช้งาน
    } else {
      server.send(500, "text/plain", "Failed to open profile.jpg"); // กรณีเปิดไฟล์ไม่สำเร็จ
    }
  } else {
    server.send(404, "text/plain", "File not found"); // กรณีไม่พบไฟล์
  }
}

void handleBgPicture() {
  // ตรวจสอบว่าไฟล์ profile.jpg มีอยู่ใน SD Card หรือไม่
  if (SD.exists("/" + firmwareVersion + "/system/website/bg.jpg")) {
    File file = SD.open("/" + firmwareVersion + "/system/website/bg.jpg", FILE_READ); // เปิดไฟล์ในโหมดอ่าน
    if (file) {
      server.streamFile(file, "image/jpeg"); // ส่งไฟล์ไปยังไคลเอนต์
      file.close(); // ปิดไฟล์หลังการใช้งาน
    } else {
      server.send(500, "text/plain", "Failed to open profile.jpg"); // กรณีเปิดไฟล์ไม่สำเร็จ
    }
  } else {
    server.send(404, "text/plain", "File not found"); // กรณีไม่พบไฟล์
  }
}

void handleProfileInfo() {
  if (LittleFS.exists("/profile.json")) {
    File file = LittleFS.open("/profile.json", FILE_READ);
    if (!file) {
      server.send(500, "application/json", "{\"error\":\"Failed to open profile.json\"}");
      return;
    }

    DynamicJsonDocument doc(2048);
    DeserializationError error = deserializeJson(doc, file);
    file.close();

    if (error) {
      server.send(500, "application/json", "{\"error\":\"Failed to parse profile.json\"}");
      return;
    }

    // เตรียม JSON ตอบกลับให้เหมือน SD format เดิม
    DynamicJsonDocument response(1024);
    JsonObject profileOut = response.createNestedObject("profile");

    profileOut["firstname"] = doc["profile"]["firstname"] | "";
    profileOut["lastname"]  = doc["profile"]["lastname"]  | "";
    profileOut["nickname"]  = doc["profile"]["nickname"]  | "";
    profileOut["position"]  = doc["profile"]["position"]  | "";
    profileOut["company"]   = doc["profile"]["company"]   | "";
    profileOut["motto"]     = doc["profile"]["motto"]     | "";

    JsonObject colors = profileOut.createNestedObject("colors");
    colors["background"] = doc["colors"]["background"] | "#000000";
    colors["name"]       = doc["colors"]["name"]       | "#FFAE00";
    colors["detail"]     = doc["colors"]["detail"]     | "#FFFFFF";
    colors["motto"]      = doc["colors"]["motto"]      | "#FFAE00";
    colors["dateTime"]   = doc["colors"]["datetime"]   | "#FFFFFF";

    JsonObject contacts = profileOut.createNestedObject("contacts");
    contacts["facebook"]  = doc["social"]["facebook"]       | "";
    contacts["line"]      = doc["contact"]["line"]          | "";
    contacts["instagram"] = doc["social"]["instagram"]      | "";
    contacts["youtube"]   = doc["social"]["youtube"]        | "";
    contacts["telephone"] = doc["contact"]["tel"]           | "";
    contacts["email"]     = doc["contact"]["email"]         | "";
    contacts["tiktok"]    = doc["social"]["tiktok"]         | "";
    contacts["whatsapp"]  = doc["contact"]["whatsapp"]      | "";

    JsonObject wallet = response.createNestedObject("wallet");
    wallet["usdt"]      = doc["wallet"]["usdt"]      | "0";
    wallet["btc"]       = doc["wallet"]["btc"]       | "0";
    wallet["cct"]       = doc["wallet"]["cct"]       | "0";
    wallet["stripe"]    = doc["wallet"]["stripe"]    | "0";
    wallet["promptpay"] = doc["wallet"]["promptpay"] | "0000000000";


    // ส่ง JSON กลับ
    String jsonResponse;
    serializeJson(response, jsonResponse);
    server.send(200, "application/json", jsonResponse);
  } else {
    server.send(404, "application/json", "{\"error\":\"Profile info not found in LittleFS.\"}");
  }
}




void handleInitialValues() {
  DynamicJsonDocument doc(4096);

  doc["pageMode"] = pageMode;
  doc["profileDuration"] = profileDuration;
  doc["profileToggle"] = profileToggle;
  doc["coinDuration"] = coinDuration;
  doc["coinToggle"] = coinToggle;
  doc["screensaverDuration"] = screensaverDuration;
  doc["screensaverToggle"] = screensaverToggle;
  doc["cdcDuration"] = cdcDuration;
  doc["cdcToggle"] = cdcToggle;
  doc["lotteryAlert"] = lotteryAlert;

  // เพิ่ม coin1, coin2, coin3, coin4 และข้อมูลที่เกี่ยวข้องลงใน response
  for (int i = 0; i < 4; i++) {
    doc["coin" + String(i + 1)] = coin[i];
    doc["type" + String(i + 1)] = type[i];
    doc["color" + String(i + 1)] = color[i];
    doc["market" + String(i + 1)] = market[i];  // ✅ เพิ่ม market[i] ด้วย
  }

  doc["sdStatus"] = sdStatus;
  doc["timeZone"] = timeZone;
  doc["region"] = region;
  doc["currency1"] = currency1;
  doc["currency2"] = currency2;

  String jsonResponse;
  serializeJson(doc, jsonResponse);
  server.send(200, "application/json", jsonResponse);
}

void handleInitialAlerts() {
  DynamicJsonDocument json(8192);
  json["currency"] = currencyAlert;
  json["lotteryAlert"] = lotteryAlert;
  json["alertEnable"] = alertEnable ;
  JsonArray alerts = json.createNestedArray("alerts");

  for (int i = 0; i < 4; i++) {
    JsonObject item = alerts.createNestedObject();
    item["coinIndex"] = i;

    JsonArray high = item.createNestedArray("high");
    for (int j = 0; j < alertCountHigh[i]; j++) {
      JsonObject h = high.createNestedObject();
      h["value"] = coinHigh[i][j];
      h["status"] = coinHighStatus[i][j];
    }

    JsonArray low = item.createNestedArray("low");
    for (int j = 0; j < alertCountLow[i]; j++) {
      JsonObject l = low.createNestedObject();
      l["value"] = coinLow[i][j];
      l["status"] = coinLowStatus[i][j];
    }
  }

  String output;
  serializeJson(json, output);
  server.send(200, "application/json", output);
}



void setupWiFiAPMode() {
  // เปิดใช้งาน AP Mode
  WiFi.softAP("CryptoClock");
  Serial.println("Entered AP Mode. SSID: CryptoClock");

  // เส้นทาง "/" ส่งหน้า Index
  server.on("/", HTTP_GET, handleIndex1);

  // เส้นทาง "/scan-wifi" ส่งข้อมูล WiFi ที่สแกนเจอในรูปแบบ JSON
  server.on("/scan-wifi", HTTP_GET, []() {
    server.send(200, "application/json", jsonWiFi);
  });

  // เส้นทาง "/upload" บันทึก SSID และ Password ลง JSON
  server.on("/upload", HTTP_POST, []() {
    wifi_ssid = server.arg("ssid");
    wifi_password = server.arg("password");

    if (wifi_ssid.isEmpty() || wifi_password.isEmpty()) {
      server.send(400, "text/html", "SSID or Password cannot be empty");
      return;
    }

    // บันทึกการตั้งค่า WiFi ลง JSON
    if (save_wifi()) {
      // ส่งข้อความยืนยันไปยังไคลเอนต์
      server.send(200, "text/html", "<!DOCTYPE html><html><head><meta http-equiv='refresh' content='10;url=/'></head><body>Settings saved. Restarting...</body></html>");
      delay(1000);
      ESP.restart();
    } else {
      server.send(500, "text/html", "Failed to open config file for writing");
      Serial.println("Failed to save WiFi settings");
    }
  });

  // เส้นทาง Not Found
  server.onNotFound(handleNotFound);

  // เริ่มต้นเซิร์ฟเวอร์
  server.begin();
  Serial.println("HTTP server started");
}




bool downloadImage(String filename, const char* url) {
  String fullname = "/" + firmwareVersion + "/system/image/" + filename + ".jpg"; // กำหนดตำแหน่งไฟล์ใน SD Card
  bool downloadSuccessful = false;
  int attempts = 0;
  const int maxAttempts = 10; // จำนวนครั้งสูงสุดที่ต้องการลองดาวน์โหลด



  while (!downloadSuccessful && attempts < maxAttempts) {
    http3.begin(url); // เริ่มการเชื่อมต่อ HTTP
    int httpCode = http3.GET();

    if (httpCode == 200) {
      int contentLength = http3.getSize();
      Serial.print("Expected Content Length: ");
      Serial.println(contentLength);

      File file = SD.open(fullname, FILE_WRITE); // เปิดไฟล์สำหรับเขียน
      if (!file) {
        Serial.println("Failed to open file for writing");
        http3.end();
        return false;
      }

      // เขียนข้อมูลลงในไฟล์
      int writtenBytes = http3.writeToStream(&file);
      file.close();

      if (writtenBytes == contentLength) {
        Serial.println("Image saved to SD Card with correct size");
        downloadSuccessful = true; // ดาวน์โหลดสำเร็จ
      } else {
        Serial.println("Downloaded image size mismatch");
        Serial.print("SD Card Content Length: ");
        Serial.println(writtenBytes);
        SD.remove(fullname); // ลบไฟล์ที่ไม่สมบูรณ์
      }
    } else {
      Serial.println("Failed to download the image, attempt " + String(attempts + 1));
    }

    http3.end(); // ปิดการเชื่อมต่อ HTTP
    attempts++; // เพิ่มจำนวนครั้งที่ลองดาวน์โหลด

    if (!downloadSuccessful) {
      delay(2000); // หน่วงเวลาก่อนลองดาวน์โหลดอีกครั้ง
    }
  }

  return downloadSuccessful;
}



//screen saver



String listFiles() {
  String path = "/" + firmwareVersion ;
  File root = SD.open(path);
  if (!root) {
    Serial.println("❌ SD.open(path) failed. Path = " + path);
    return "{\"files\":[]}";
  }
  Serial.print("List Screen Saver File At : ");
  Serial.println(path);
  DynamicJsonDocument doc(2048);
  JsonArray filesArray = doc.createNestedArray("files");

  // Array to store file names temporarily
  String fileNames[10];
  fileCount = 0;

  while (true) {
    File entry = root.openNextFile();
    if (!entry) {
      break;
    }
    String fileName = entry.name();

    if (fileName.startsWith("bg") && fileName.endsWith(".jpg")) {
      int index = fileName.substring(3, 6).toInt();
      if (index >= 1 && index <= 10) {
        fileNames[index - 1] = fileName;
        fileCount++;
      }
    }
    entry.close();
  }
  root.close();

  // Add file names to JSON array in sorted order
  for (int i = 0; i < fileCount; i++) {
    if (fileNames[i].length() > 0) {
      filesArray.add("/" + firmwareVersion + "/" + fileNames[i]);
    }
  }

  String output;
  serializeJson(doc, output);
  return output;
}

String formatFileName(int num) {
  char filename[18];
  //snprintf(filename, sizeof(filename), "/bg%03d.jpg", num);
  snprintf(filename, sizeof(filename), "/%s/bg%03d.jpg", firmwareVersion.c_str(), num);
  return String(filename);
}

String getContentType(String filename) {
  if (server.hasArg("download")) return "application/octet-stream";
  else if (filename.endsWith(".htm")) return "text/html";
  else if (filename.endsWith(".html")) return "text/html";
  else if (filename.endsWith(".css")) return "text/css";
  else if (filename.endsWith(".js")) return "application/javascript";
  else if (filename.endsWith(".png")) return "image/png";
  else if (filename.endsWith(".gif")) return "image/gif";
  else if (filename.endsWith(".jpg")) return "image/jpeg";

  return "text/plain";
}

void deleteFile(String path) {
  path = "/" + path;
  if (SD.exists(path)) {
    SD.remove(path);
    Serial.println("File deleted: " + path);
    reorderFiles(path); // เรียกฟังก์ชันเพื่อเรียงไฟล์ใหม่หลังจากลบไฟล์
  } else {
    Serial.println("File not found: " + path);
  }
}

void reorderFiles(String deletedFileName) {
  int deletedIndex = deletedFileName.substring(3, 6).toInt();
  String fileName;

  for (int i = deletedIndex + 1; i <= 10; i++) {
    String oldFileName = formatFileName(i);
    String newFileName = formatFileName(i - 1);
    if (SD.exists(oldFileName)) {
      SD.rename(oldFileName, newFileName);
      Serial.println("Renamed " + oldFileName + " to " + newFileName);
    } else {
      break;
    }
  }
}



void handleDeleteFile() {
  if (server.hasArg("path")) {
    String path = server.arg("path");
    Serial.println("Delete request for: " + path);
    deleteFile(path); // ลบไฟล์และเรียงลำดับไฟล์ใหม่
    server.send(200, "text/plain", "File deleted and reordered");
  } else {
    server.send(400, "text/plain", "Path not specified");
  }
}

void handleFileFromSD() {
  String path = server.uri(); // Get the path from the URL
  Serial.println("Request to serve file: " + path);

  if (SD.exists(path)) {
    File file = SD.open(path, FILE_READ);
    server.streamFile(file, getContentType(path));
    file.close();
  } else {
    Serial.println("File not found: " + path);
    server.send(404, "text/plain", "File not found");
  }
}

void handleNotFound() {
  String path = server.uri(); // Get the path from the URL
  Serial.println("Not found: " + path);

  // Check if the requested path matches the pattern bg*.jpg
  if (path.indexOf("/bg") != -1 && path.endsWith(".jpg")) {
    handleFileFromSD();
  } else {
    String message = "File Not Found\n\n";
    message += "URI: ";
    message += server.uri();
    message += "\nMethod: ";
    message += (server.method() == HTTP_GET) ? "GET" : "POST";
    message += "\nArguments: ";
    message += server.args();
    message += "\n";
    for (uint8_t i = 0; i < server.args(); i++) {
      message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
    }
    server.send(404, "text/plain", message);
  }
}

void handleRoot2() {
  // เปิดไฟล์จากตำแหน่ง /" + firmwareVersion +"/system/website/gallery.html บน SD Card
  File file = SD.open("/" + firmwareVersion + "/system/website/gallery.html", FILE_READ);
  if (!file) {
    server.send(404, "text/plain", "File not found");
    Serial.println("File /" + firmwareVersion + "/system/website/gallery.html not found on SD Card");
    return;
  }

  // อ่านเนื้อหาไฟล์ HTML
  String html = file.readString();
  file.close();

  // ส่งเนื้อหา HTML ไปยังไคลเอนต์
  server.send(200, "text/html", html);
  Serial.println("File /" + firmwareVersion + "/system/website/gallery.html served successfully");
}


void handleListFiles() {
  server.send(200, "application/json", listFiles());
}


int countFiles() {
  File root = SD.open("/" + firmwareVersion);
  if (!root) {
    Serial.println("Failed to open root directory");
    return 0;
  }

  int fileCount = 0;
  Serial.println("Start SD");
  root.rewindDirectory();  // รีเซ็ตการอ่านไฟล์จากจุดเริ่มต้น
  File entry;
  while ((entry = root.openNextFile())) {
    Serial.print("Entry name: ");
    Serial.println(entry.name());
    if (!entry.isDirectory()) {  // ตรวจสอบว่าเป็นไฟล์ไม่ใช่ directory
      String fileName = entry.name();
      Serial.print("Filename Read: ");
      Serial.println(fileName);
      if (fileName.startsWith("bg") && fileName.endsWith(".jpg")) {
        fileCount++;
      }
    } else {
      //Serial.println("Entry is a directory, skipping...");
    }
    entry.close();
  }

  Serial.println("Close SD");
  root.close();
  Serial.print("fileCount: ");
  Serial.println(fileCount);
  return fileCount;
}

void handleFileUpload2() {
  HTTPUpload& upload = server.upload();
  static int nextFileNum = 1; // ตัวแปรเพื่อเก็บหมายเลขไฟล์ต่อไปที่จะอัพโหลด

  if (upload.status == UPLOAD_FILE_START) {
    fileCount = countFiles(); // นับจำนวนไฟล์ใน SD

    if (fileCount >= 10) { // ตรวจสอบจำนวนไฟล์สูงสุด
      Serial.println("Maximum");
      server.send(400, "text/plain", "Maximum of 10 files can be uploaded.");
      return;
    }

    // ค้นหาไฟล์ต่อไปที่จะอัพโหลด
    for (int i = 1; i <= 10; i++) {
      String filename = formatFileName(i);
      if (!SD.exists(filename)) {
        nextFileNum = i;
        break;
      }
    }

    String filename = formatFileName(nextFileNum);
    Serial.printf("UploadStart: %s\n", filename.c_str());
    uploadFile = SD.open(filename, FILE_WRITE);
  } else if (upload.status == UPLOAD_FILE_WRITE) {
    if (uploadFile) {
      uploadFile.write(upload.buf, upload.currentSize);
    }
  } else if (upload.status == UPLOAD_FILE_END) {
    if (uploadFile) {
      uploadFile.close();

      delay(10); // Small delay to ensure the file is closed properly
      Serial.printf("UploadEnd: %s\n", upload.filename.c_str());
    }
  } else if (upload.status == UPLOAD_FILE_ABORTED) {
    if (uploadFile) {
      uploadFile.close();
      SD.remove(formatFileName(nextFileNum)); // Remove the incomplete file
      Serial.printf("Upload aborted\n");
    }
  }
}

void handleUpdateOrder() {
  Serial.println("Update Order");
  if (server.hasArg("plain")) {
    DynamicJsonDocument doc(2048);
    DeserializationError error = deserializeJson(doc, server.arg("plain"));
    if (error) {
      server.send(400, "text/plain", "Invalid JSON");
      return;
    }

    JsonArray order = doc["order"];
    String tempFileNames[order.size()];

    // Step 1: Rename all files to temporary names
    for (size_t i = 0; i < order.size(); i++) {
      String oldFileName = order[i].as<String>();

      // ⭐ ตัด path เหลือเฉพาะชื่อไฟล์
      int slash = oldFileName.lastIndexOf('/');
      if (slash >= 0) {
        oldFileName = oldFileName.substring(slash + 1);
      }
      oldFileName = "/" + firmwareVersion + "/" + oldFileName;

      String tempFileName = oldFileName + ".tmp";
      tempFileNames[i] = tempFileName;
      if (SD.exists(oldFileName)) {
        SD.rename(oldFileName, tempFileName);
        Serial.printf("Renamed %s to %s\n", oldFileName.c_str(), tempFileName.c_str());
      }
    }

    // Step 2: Rename temporary files to final new names
    for (size_t i = 0; i < order.size(); i++) {
      String tempFileName = tempFileNames[i];
      String newFileName = formatFileName(i + 1);
      if (SD.exists(tempFileName)) {
        SD.rename(tempFileName, newFileName);
        Serial.printf("Renamed %s to %s\n", tempFileName.c_str(), newFileName.c_str());
      }
    }
    server.send(200, "text/plain", "Order updated");
  } else {
    server.send(400, "text/plain", "No JSON data");
  }
}


//end screensaver

// 📌 ฟังก์ชันโหลด `api_config.json`
void loadAPIConfig() {
  if (!SD.exists("/" + firmwareVersion + "/system/website/api_config.json")) {
    Serial.println("No API config found, using default values.");
    return;
  }

  File file = SD.open("/" + firmwareVersion + "/system/website/api_config.json");
  if (!file) {
    Serial.println("Failed to open API config file!");
    return;
  }

  StaticJsonDocument<1024> doc;
  DeserializationError error = deserializeJson(doc, file);
  file.close();

  if (error) {
    Serial.println("Failed to parse JSON");
    return;
  }
  // โหลดค่า config ของ currency1 และ currency2 ลงใน array
  for (int i = 0; i < 2; i++) {
    api_url[i]    = doc[configKeys[i]]["url"].as<String>();
    price_path[i] = doc[configKeys[i]]["json_path"]["price"].as<String>();
    high_path[i]  = doc[configKeys[i]]["json_path"]["highPrice"].as<String>();
    low_path[i]   = doc[configKeys[i]]["json_path"]["lowPrice"].as<String>();
    change_path[i] = doc[configKeys[i]]["json_path"]["priceChange"].as<String>();
    percent_path[i] = doc[configKeys[i]]["json_path"]["priceChangePercent"].as<String>();
  }

  Serial.println("Loaded API Configuration:");
  Serial.print("currency1: ");
  Serial.println(api_url[0]);
  Serial.print("currency2: ");
  Serial.println(api_url[1]);
}

// 📌 ฟังก์ชันบันทึกค่า API ใหม่จากหน้าเว็บ
void handleSaveConfig() {
  if (!server.hasArg("plain")) {
    server.send(400, "text/plain", "Bad Request");
    return;
  }

  String jsonString = server.arg("plain");

  File file = SD.open("/" + firmwareVersion + "/system/website/api_config.json", FILE_WRITE);
  if (!file) {
    server.send(500, "text/plain", "Failed to open config file.");
    return;
  }

  file.print(jsonString); // ✅ บันทึก JSON ใหม่ลงไฟล์
  file.close();

  loadAPIConfig(); // ✅ โหลดค่าใหม่ทันที
  server.send(200, "text/plain", "Configuration updated.");
  Serial.println("API Config updated from Web.");
  ESP.restart();
}

void setupWebServer() {
  Serial.print("Free Heap: ");
  Serial.println(ESP.getFreeHeap());
  serverOn = true;
  // Handle root URL
  server.on("/", HTTP_GET, []() {
    // เปิดไฟล์จากตำแหน่ง /" + firmwareVersion +"/system/website/settings.html บน SD Card
    File file = SD.open("/" + firmwareVersion + "/system/website/settings.html", FILE_READ);
    if (!file) {
      Serial.println("Failed to open /" + firmwareVersion + "/system/website/settings.html");
      server.send(500, "text/plain", "Failed to open file");
      return;
    }

    Serial.println("Sending file...");
    server.sendHeader("Cache-Control", "no-store, no-cache, must-revalidate, max-age=0");
    server.sendHeader("Pragma", "no-cache");
    server.sendHeader("Expires", "0");
    // ส่งไฟล์ไปยังไคลเอนต์
    server.streamFile(file, "text/html");
    file.close();
    Serial.println("File sent successfully");
  });


  server.on("/bg.jpg", HTTP_GET, handleBgPicture);

  // Handle profile picture
  server.on("/profile.jpg", HTTP_GET, handleProfilePicture);

  // Handle profile info
  server.on("/profile-info", HTTP_GET, handleProfileInfo);

  // Handle initial values
  server.on("/initial-values", HTTP_GET, handleInitialValues);

  // Handle alerts  values
  server.on("/initial-alerts", HTTP_GET, handleInitialAlerts);

  File file = SD.open("/" + firmwareVersion + "/system/website/api.html");
  if (!file) {
    server.send(500, "text/plain", "Failed to open api.html");
    return;
  }

  server.on("/submit-profile", HTTP_POST, []() {
    Serial.println("Start Profile Submission");

    // 🧍‍♂️ ข้อมูล profile
    profile_firstname = server.hasArg("firstname") ? server.arg("firstname") : "";
    profile_lastname  = server.hasArg("lastname")  ? server.arg("lastname")  : "";
    profile_nickname  = server.hasArg("nickname")  ? server.arg("nickname")  : "";
    profile_position  = server.hasArg("position")  ? server.arg("position")  : "";
    profile_company   = server.hasArg("company")   ? server.arg("company")   : "";
    profile_motto     = server.hasArg("motto")     ? server.arg("motto")     : "";

    // 🎨 สี
    profile_bgcolor       = server.hasArg("bgcolor")       ? server.arg("bgcolor")       : "#FFFFFF";
    profile_namecolor     = server.hasArg("namecolor")     ? server.arg("namecolor")     : "#000000";
    profile_detailcolor   = server.hasArg("detailcolor")   ? server.arg("detailcolor")   : "#000000";
    profile_mottocolor    = server.hasArg("mottocolor")    ? server.arg("mottocolor")    : "#000000";
    profile_datetimecolor = server.hasArg("datetimecolor") ? server.arg("datetimecolor") : "#000000";

    // 📱 Contact
    contact_line     = server.hasArg("contact_line")     ? server.arg("contact_line")     : "0";
    contact_whatsapp = server.hasArg("contact_whatsapp") ? server.arg("contact_whatsapp") : "0";
    contact_tel      = server.hasArg("contact_tel")      ? server.arg("contact_tel")      : "0";
    contact_email    = server.hasArg("contact_email")    ? server.arg("contact_email")    : "mailto:mycryptoclock@gmail.com";

    // 🌐 Social
    social_facebook  = server.hasArg("social_facebook")  ? server.arg("social_facebook")  : "";
    social_instagram = server.hasArg("social_instagram") ? server.arg("social_instagram") : "";
    social_youtube   = server.hasArg("social_youtube")   ? server.arg("social_youtube")   : "";
    social_tiktok    = server.hasArg("social_tiktok")    ? server.arg("social_tiktok")    : "";

    // ค่าพิเศษ
    profile_refer = server.hasArg("refer") ? server.arg("refer") : "CCN000000";
    profile_referring.clear(); // เคลียร์ไว้ก่อน เผื่อมี future use

    region   = server.hasArg("region")   ? server.arg("region")   : "default";
    timeZone = server.hasArg("timeZone") ? server.arg("timeZone") : "Asia/Bangkok";
    pageIndex = 2;
    //save region & timeZone
    if (save_config()) {
      Serial.println("Region & Timezone saved to config.json");
    } else {
      Serial.println("Failed to save region/timeZone to config.json");
    }

    // 💾 เซฟโปรไฟล์
    if (save_profile()) {
      Serial.println("Profile saved successfully to /profile.json");
      server.send(200, "text/html", "<!DOCTYPE html><html><head><meta http-equiv='refresh' content='5;url=/'></head><body>Profile Submitted Successfully</body></html>");
      delay(1000);
      ESP.restart();
    } else {
      Serial.println("Failed to save profile");
      server.send(500, "text/html", "Failed to save profile");
    }

    //ssave setting

  }, handleFileUpload);

  server.on("/submit-coins", HTTP_POST, []() {
    Serial.println("Start Coin Selection Submission");

    for (int i = 0; i < 4; i++) {
      const String idx        = String(i + 1);
      const String coinKey    = "coin"   + idx;
      const String typeKey    = "type"   + idx;
      const String colorKey   = "color"  + idx;
      const String marketKey  = "market" + idx;

      coin[i] = server.hasArg(coinKey) ? server.arg(coinKey) : "";
      type[i] = server.hasArg(typeKey) ? server.arg(typeKey) : "3";
      color[i] = server.hasArg(colorKey) ? server.arg(colorKey) : "#FFFFFF";
      market[i] = server.hasArg(marketKey) ? server.arg(marketKey) : "NASDAQ";
      Serial.printf("[SUBMIT] coin%d = %s, type = %s, color = %s, market = %s\n",
                    i + 1,
                    coin[i].c_str(),
                    type[i].c_str(),
                    color[i].c_str(),
                    market[i].c_str());
    }

    // 🔽 รับค่า currency1 และ currency2
    currency1 = server.hasArg("currency1") ? server.arg("currency1") : "USDT";
    currency2 = server.hasArg("currency2") ? server.arg("currency2") : "THB";
    Serial.printf("[SUBMIT] currency1 = %s, currency2 = %s\n", currency1.c_str(), currency2.c_str());
    pageIndex = 2;
    save_config();
    // บันทึกลง LittleFS
    save_coin();
    downloadAllCoinIcons(false);

    server.send(200, F("text/html"), F("Coin Selection Submitted Successfully"));
    Serial.println("===== Reboot =====");
    ESP.restart();
  });



  server.on("/submit-alerts", HTTP_POST, []() {
    Serial.println("📥 [SUBMIT ALERTS] เริ่มรับข้อมูล POST");

    currencyAlert = server.arg("currency");
    String arg = server.arg("alertEnable");
    alertEnable = server.hasArg("alertEnable") && (arg == "true" || arg == "on");
    lotteryAlert = server.hasArg("lotteryAlert") && (arg == "true" || arg == "on");

    Serial.print("🪙 currencyAlert = ");
    Serial.println(currencyAlert);
    Serial.print("✅ alertEnable = ");
    Serial.println(alertEnable ? "true" : "false");
    Serial.print("🎰 lotteryAlert = ");
    Serial.println(lotteryAlert ? "true" : "false");

    for (int i = 0; i < MAX_COINS; i++) {
      alertCountHigh[i] = 0;
      alertCountLow[i] = 0;

      Serial.printf("🔍 Coin %d:\n", i);

      // 🔺 High loop
      for (int j = 0; j < MAX_ALERTS; j++) {
        String key = "high_" + String(i) + "_" + String(j);
        String statusKey = "status_high_" + String(i) + "_" + String(j);

        if (!server.hasArg(key)) {
          Serial.printf("  🚫 ไม่พบ high_%d_%d\n", i, j);
          break;
        }

        coinHigh[i][j] = server.arg(key).toFloat();
        coinHighStatus[i][j] = server.hasArg(statusKey);
        alertCountHigh[i]++;

        Serial.printf("  ✅ HIGH[%d][%d] = %.6f (status = %s)\n",
                      i, j, coinHigh[i][j], coinHighStatus[i][j] ? "true" : "false");
      }

      // 🔻 Low loop
      for (int j = 0; j < MAX_ALERTS; j++) {
        String key = "low_" + String(i) + "_" + String(j);
        String statusKey = "status_low_" + String(i) + "_" + String(j);

        if (!server.hasArg(key)) {
          Serial.printf("  🚫 ไม่พบ low_%d_%d\n", i, j);
          break;
        }

        coinLow[i][j] = server.arg(key).toFloat();
        coinLowStatus[i][j] = server.hasArg(statusKey);
        alertCountLow[i]++;

        Serial.printf("  ✅ LOW[%d][%d] = %.6f (status = %s)\n",
                      i, j, coinLow[i][j], coinLowStatus[i][j] ? "true" : "false");
      }

      // ✨ เคลียร์ส่วนที่เหลือ
      for (int j = alertCountHigh[i]; j < MAX_ALERTS; j++) {
        coinHigh[i][j] = 10000000.0;
        coinHighStatus[i][j] = false;
      }
      for (int j = alertCountLow[i]; j < MAX_ALERTS; j++) {
        coinLow[i][j] = 0.0;
        coinLowStatus[i][j] = false;
      }

      Serial.printf("  🎯 High count = %d, Low count = %d\n", alertCountHigh[i], alertCountLow[i]);
    }

    pageIndex = 2;
    save_config();

    if (save_alert()) {
      Serial.println("✅ Alerts saved successfully.");
      server.send(200, "text/plain", "Alert saved");
      ESP.restart();
    } else {
      Serial.println("❌ Failed to save alerts.");
      server.send(500, "text/plain", "Save failed");
    }
  });


  // Handle page setup submission
  server.on("/submit-page", HTTP_POST, []() {
    Serial.println("Start Page Setup Submission");

    // Always assign with default fallback if empty
    pageMode = server.arg("pageModeValue1");
    if (pageMode == "") pageMode = "dynamic";

    profileDuration = server.arg("profileDuration");
    if (profileDuration == "") profileDuration = "15";

    profileToggle = server.arg("profileToggleValue1") == "true"; // ไม่มี toggle ก็คือ false อยู่แล้ว

    coinDuration = server.arg("coinDuration");
    if (coinDuration == "") coinDuration = "15";

    coinToggle = server.arg("coinToggleValue1") == "true";

    screensaverDuration = server.arg("screensaverDuration");
    if (screensaverDuration == "") screensaverDuration = "30";

    screensaverToggle = server.arg("screensaverToggleValue1") == "true";

    cdcDuration = server.arg("cdcDuration");
    if (cdcDuration == "") cdcDuration = "15";

    cdcToggle = server.arg("cdcToggleValue1") == "true";

    // ตั้งค่า pageIndex และเซฟ config
    pageIndex = 2;
    save_config();
    delay(200);

    server.send(200, "text/html", "Page Setup Submitted Successfully");
    ESP.restart();
  });




  // Handle 404 not found
  server.onNotFound(handleNotFound);

  //screen saver
  // Handle gallery.html
  server.on("/gallery.html", HTTP_GET, handleGallery);
  server.on("/listfiles", HTTP_GET, handleListFiles);
  server.on("/upload", HTTP_POST, []() {
    // This part handles the response for the file upload
    server.send(200, "text/plain", "File Uploaded Successfully");
  }, handleFileUpload2);
  server.on("/deletefile", HTTP_GET, handleDeleteFile);
  server.on("/updateOrder", HTTP_POST, handleUpdateOrder);

  //api
  server.on("/api.html", HTTP_GET, []() {

    File file = SD.open("/" + firmwareVersion + "/system/website/api.html");
    if (!file) {
      server.send(500, "text/plain", "Failed to open api.html");
      return;
    }

    String htmlContent;
    while (file.available()) {
      htmlContent += (char)file.read();
    }
    file.close();

    server.send(200, "text/html", htmlContent);
  });
  server.on("/getConfig", HTTP_GET, []() {
    if (!SD.exists("/" + firmwareVersion + "/system/website/api_config.json")) {
      server.send(404, "text/plain", "Config file not found.");
      return;
    }

    File file = SD.open("/" + firmwareVersion + "/system/website/api_config.json");
    if (!file) {
      server.send(500, "text/plain", "Failed to open config file.");
      return;
    }

    String jsonContent;
    while (file.available()) {
      jsonContent += (char)file.read();
    }
    file.close();

    server.send(200, "application/json", jsonContent);
  });
  server.on("/saveConfig", HTTP_POST, handleSaveConfig); // ✅ เพิ่ม API ให้บันทึกค่า API

  File fsUploadFile; // Global

  // ✅ สำหรับ WebServer (Sync Server)
  server.on("/upload-settings", HTTP_POST, []() {
    Serial.println("Finish Upload settings.html");
    server.send(200, "text/html", "<html><body><h3>Upload complete. Rebooting...</h3><script>setTimeout(function(){location.href='/'},3000);</script></body></html>");
    delay(1000);
    ESP.restart();
  }, handleSettingsUpload);





  // Start the server
  server.begin();
  Serial.println("HTTP server started");
}


void showQRWeb()
{
  tft.setTextSize(2);
  tft.setCursor(5, 5);
  tft.print("Connect same WiFi");
  tft.setCursor(5, 40);
  tft.setTextSize(1);
  tft.print(newurl);
  drawQRCode(newurl2.c_str(), 4, 5 , 30);


}

String adsValue = "" ;

char strftime_buf[64];

String convertTimeZone() {
  if (timeZone == "Europe/London") return "GMT0BST,M3.5.0/1,M10.5.0";  // London
  else if (timeZone == "Europe/Paris") return "CET-1CEST,M3.5.0/2,M10.5.0/3";  // Paris
  else if (timeZone == "Europe/Athens") return "EET-2EEST,M3.5.0/3,M10.5.0/4";  // Athens
  else if (timeZone == "Asia/Baghdad") return "AST-3";  // Baghdad
  else if (timeZone == "Asia/Tehran") return "IRST-3:30";  // Tehran
  else if (timeZone == "Asia/Dubai") return "GST-4";  // Abu Dhabi
  else if (timeZone == "Asia/Kabul") return "AFT-4:30";  // Kabul
  else if (timeZone == "Asia/Karachi") return "PKT-5";  // Islamabad
  else if (timeZone == "Asia/Kolkata") return "IST-5:30";  // Mumbai
  else if (timeZone == "Asia/Kathmandu") return "NPT-5:45";  // Kathmandu
  else if (timeZone == "Asia/Dhaka") return "BST-6";  // Dhaka
  else if (timeZone == "Asia/Bangkok") return "ICT-7";  // Bangkok
  else if (timeZone == "Asia/Hong_Kong") return "HKT-8";  // Hong Kong
  else if (timeZone == "Asia/Tokyo") return "JST-9";  // Tokyo
  else if (timeZone == "Australia/Sydney") return "AEST-10AEDT,M10.1.0/2,M4.1.0/3";  // Sydney
  else if (timeZone == "Pacific/Fiji") return "FJT-12";  // Fiji
  else if (timeZone == "Pacific/Tongatapu") return "TOT-13";  // Nuku'alofa

  // Default case
  return timeZone;
}



// ตั้งค่าเวลาผ่าน NTP
void setupTime() {
  configTime(0, 0, "pool.ntp.org", "time.nist.gov");
  String tz = convertTimeZone();
  setenv("TZ", tz.c_str(), 1);
  Serial.print("Settime To :");
  Serial.println(timeZone);
  tzset();
}

void initialParameter()
{
  // Cloud transport and OTA topics are disabled in this local build.
}




void nextpage()
{
  const int pageSequence[] = {1, 2, 7, 26}; // ลำดับหน้า
  const int pageSequenceCount = sizeof(pageSequence) / sizeof(pageSequence[0]);
  bool pageSuccess = false ;

  if (millis() - pageChangeTime >= pageDurations[pageSequenceIndex]) {
    Serial.println("next");
    Serial.print("pageSequenceIndex: "); Serial.println(pageSequenceIndex);
    Serial.print("page: "); Serial.println(pageSequence[pageSequenceIndex]);
    Serial.print("coinIndex: "); Serial.println(coinIndex);
    Serial.print("coinType: "); Serial.println(coinType);
    Serial.print("profileToggle: "); Serial.println(profileToggle);
    Serial.print("coinToggle: "); Serial.println(coinToggle);
    Serial.print("screensaverToggle: "); Serial.println(screensaverToggle);
    Serial.print("cdcToggle: "); Serial.println(cdcToggle);
    Serial.print("sdStatus: "); Serial.println(sdStatus);
    pageChangeTime = millis();
    while (!pageSuccess)
    {
      if (pageSequence[pageSequenceIndex] == 2 && coinIndex != maxCoin - 1)
      {
        coinIndex = (coinIndex + 1 ) % maxCoin    ;
        coinType = type[coinIndex].toInt();
        updateDisplayValues(); // ⭐️ เรียกตรงนี้ทันทีหลังเปลี่ยนเหรียญ

      }
      else  if (pageSequence[pageSequenceIndex] == 1 && coinIndex != 0 )
      {
        coinIndex = 0 ;
        coinType = type[coinIndex].toInt();

        pageSequenceIndex = (pageSequenceIndex + 1) % pageSequenceCount; // page 2
      }
      else
      {
        pageSequenceIndex = (pageSequenceIndex + 1) % pageSequenceCount; // วนลำดับหน้า
      }
      if (pageSequence[pageSequenceIndex] == 1 && profileToggle == true)
      {
        pageSuccess = true;
      }
      else if (pageSequence[pageSequenceIndex] == 2 && coinToggle == true)
      {
        pageSuccess = true;
      }
      else if (pageSequence[pageSequenceIndex] == 7 && screensaverToggle == true && sdStatus == true)
      {
        pageSuccess = true;
      }
      else if (pageSequence[pageSequenceIndex] == 26 && cdcToggle == true)
      {
        pageSuccess = true;
      }
    }
    Serial.print("page success:");
    Serial.println(pageSequence[pageSequenceIndex]) ;

    stopPriceTask();
    silenceBuzzer(); // BEEP TEST: hold passive buzzer quiet before page redraw.
    delay(500);

    if (pageIndex == 7)
    {
      isLowPowerMode = false ;
      tft.fillScreen(TFT_BLACK);
      analogWrite(21, 255);
    }
    pageIndex = pageSequence[pageSequenceIndex];
    if (pageIndex == 1)
    {
      tft.fillScreen(TFT_BLACK);
      gotopage1();
    }
    else if (pageIndex == 2)
    {
      if (coin[coinIndex] == "GOLD")
      {
        last_bar_sell = 0;
        last_bar_buy = 0;
        last_jewelry_sell = 0;
        last_jewelry_buy = 0;
        displayGoldOnScreen();
        startPriceTask();
      }
      else if (coin[coinIndex] == "JBC")
      {
        displayBackgroundImageForPage2();
        showLogoAtTopMiddle();
        showLogoAtBottomMiddle();
        displayCoinOnScreen();
        displayPriceOnScreen(displayPrice[coinIndex]);
        displayHighLowPrice();
        startPriceTask();
        // Reset ราคาต่างๆ สำหรับ JBC
        lowPrice[coinIndex] = 0;
        highPrice[coinIndex] = 0;
        priceChange[coinIndex] = 0;
        priceChangePercent[coinIndex] = 0;
      }
      else if (coinType == 4) // ⭐️ Stock
      {
        displayBackgroundImageForPage2();
        showLogoAtTopMiddle();
        showLogoAtBottomMiddle();
        displayCoinOnScreen();
        displayPriceOnScreen(displayPrice[coinIndex]);
        displayHighLowPrice();
        startPriceTask();
      }
      else if (coinType == 7) // ⭐️ Indice
      {
        displayBackgroundImageForPage2();
        showLogoAtTopMiddle();
        showLogoAtBottomMiddle();
        displayCoinOnScreen();
        displayPriceOnScreen(displayPrice[coinIndex]);
        displayHighLowPrice();
        startPriceTask();
      }
      else // crypto อื่น ๆ (BTC, ETH, etc.)
      {
        displayBackgroundImageForPage2();
        showLogoAtTopMiddle();
        showLogoAtBottomMiddle();
        displayCoinOnScreen();
        displayPriceOnScreen(displayPrice[coinIndex]);
        displayHighLowPrice();
        startPriceTask();
      }
    }
    else if (pageIndex == 2)
    {
      if (coin[coinIndex] == "GOLD")
      {
        last_bar_sell = 0;
        last_bar_buy = 0;
        last_jewelry_sell = 0;
        last_jewelry_buy = 0;
        displayGoldOnScreen();
        startPriceTask();
      }
      else if (coin[coinIndex] == "JBC")
      {
        displayBackgroundImageForPage2();
        showLogoAtTopMiddle();
        showLogoAtBottomMiddle();
        displayCoinOnScreen();
        displayPriceOnScreen(displayPrice[coinIndex]);
        displayHighLowPrice();
        startPriceTask();
        // Reset ราคาต่างๆ สำหรับ JBC
        lowPrice[coinIndex] = 0;
        highPrice[coinIndex] = 0;
        priceChange[coinIndex] = 0;
        priceChangePercent[coinIndex] = 0;
      }
      else if (coinType == 4) // ⭐️ Stock
      {
        displayBackgroundImageForPage2();
        showLogoAtTopMiddle();
        showLogoAtBottomMiddle();
        displayCoinOnScreen();
        displayPriceOnScreen(displayPrice[coinIndex]);
        displayHighLowPrice();
        startPriceTask();
      }
      else if (coinType == 7) // ⭐️ Indice
      {
        displayBackgroundImageForPage2();
        showLogoAtTopMiddle();
        showLogoAtBottomMiddle();
        displayCoinOnScreen();
        displayPriceOnScreen(displayPrice[coinIndex]);
        displayHighLowPrice();
        startPriceTask();
      }
      else // crypto อื่น ๆ (BTC, ETH, etc.)
      {
        displayBackgroundImageForPage2();
        showLogoAtTopMiddle();
        showLogoAtBottomMiddle();
        displayCoinOnScreen();
        displayPriceOnScreen(displayPrice[coinIndex]);
        displayHighLowPrice();
        startPriceTask();
      }
    }

    else if (pageIndex == 7)
    {
      Serial.println("SlideMode");
      stopPriceTask();
      isLowPowerMode = true;
    }
    else if (pageIndex == 26)
    {
      showPage26();
      lastUpdateTime = millis();
      pageChangeTime = millis();
    }
    silenceBuzzer(); // BEEP TEST: hold passive buzzer quiet after page redraw/task start.
    //settleBuzzerAfterUiEvent();
  }
}



void displaypageIndex() {
  //Serial.println(pageIndex);
  if (pageMode == "dynamic" && serverOn == false && (pageIndex == 1 || pageIndex == 2 || pageIndex == 7 || pageIndex == 26)) {
    nextpage();
  }
  if (pageIndex == 1)
  {

    if (millis() - lastUpdateTime > updateInterval ) {
      updateTime();
      lastUpdateTime = millis();
    }
  }
  else if (pageIndex == 2)
  {
    unsigned long showDurationTask = millis() - durationTask ;
    if (showDurationTask > updateInterval + 5000)
    {
      durationTask = millis();
      Serial.print("showDurationTask:") ;
      Serial.println(showDurationTask) ;
      durationTaskCount += 1 ;
      if (durationTaskCount == 4)
      {
        ESP.restart(); // Restart the ESP32
      }
    }
    else
    {
    }
    if (millis() - lastUpdateTime > updateInterval) {
      lastUpdateTime = millis();

      if (coin[coinIndex] == "GOLD")
      {
        Serial.println("Show GOLD");
        showPage2();
      }

      else
      {
        if (lastPrice != price[coinIndex]) {
          lastPrice = price[coinIndex];
          showPage2();
        }

      }
    }
  }
  else if (pageIndex == 3)
  {
    if (millis() - lastUpdateTime > updateInterval) {
      showPage3();
      lastUpdateTime = millis();
    }
  }
  else if (pageIndex == 4)
  {
    if (page4StartTime == 0) { // If starting timer for page 4, record the time
      page4StartTime = millis();
      tft.fillScreen(TFT_BLACK);
      // Additional logic to show IP address, setup web server, etc.
      animateAndCount(3);
      showQRWeb();
      drawBlueArrow() ;
      setupWebServer();
    }
    else
    {

      // Check for timeout
      if (millis() - page4StartTime > page4Timeout) {
        Serial.println("Timeout on page 4, restarting...");
        ESP.restart(); // Restart the ESP32
      }

      // In page 4, only handle server clients, no touch or user input
      server.handleClient();
    }
  }
  else if (pageIndex == 5)
  {

    if (page5StartTime == 0) { // If starting timer for page 4, record the time
      page5StartTime = millis();
      tft.fillScreen(TFT_BLACK);
      // Additional logic to show IP address, setup web server, etc.
      animateAndCount(3);
      drawBlueArrow() ;
      showQRWeb();
      setupWebServer();
    }
    else
    {

      // Check for timeout
      if (millis() - page5StartTime > page5Timeout) {
        Serial.println("Timeout on page 5, restarting...");
        ESP.restart(); // Restart the ESP32
      }

      // In page 4, only handle server clients, no touch or user input
      server.handleClient();
    }
  }
  else if (pageIndex == 6) //page 6
  {

    delay(200) ;
  }
  else if (pageIndex == 7 && isLowPowerMode == true && fileCount > 0) // screen saver
  {
    unsigned long currentMillis = millis();
    Serial.println("pageIndex == 7 && isLowPowerMode == true");
    if (currentMillis - timer > 3 * 1000) {
      Serial.print("currentIndex:" );
      Serial.println(currentIndex);
      loadImage(currentIndex);
      currentIndex = currentIndex + 1;
      Serial.print("fileCount:") ;
      Serial.println(fileCount) ;
      Serial.print("currentIndex1:") ;
      Serial.println(currentIndex) ;
      if (currentIndex >= fileCount) {
        Serial.println("all image next page");
        pageChangeTime = 0;
        nextpage();
        currentIndex = 0;
      }
      timer = millis();
    }


    delay(100) ;
  }
  else if (pageIndex == 8)
  {

    showPage8();

  }
  else if (pageIndex == 9 )
  {
    delay(200) ;
    //showPage9();

  }
  else if (pageIndex == 26)
  {
    if (millis() - lastUpdateTime > 300000) {
      showPage26();
      lastUpdateTime = millis();
    }
  }
  else if (pageIndex == 22 && millis() - lastAlertInterval > 100)
  {
    if (isTaskOn)
    {
      Serial.println("Alert!");
      stopPriceTask();
      isTaskOn = false;
      showPage22();
      delay(1000);
    }
    else
    {
      lastAlertInterval = millis() ;
    }

    if (isOn)
    {
      tft.fillRect(80, 0, 180, 45, TFT_BLACK);
      isOn = false ;
      delay(100);
    }
    else
    {
      isOn = true ;
      // Display the "LINKS" title
      tft.setTextSize(4);
      if (alertStatus == "LOW")
      {
        tft.setTextColor(TFT_GREEN, TFT_BLACK);
      }
      else if (alertStatus == "HIGH")
      {
        tft.setTextColor(TFT_RED, TFT_BLACK);
      }

      tft.setTextDatum(ML_DATUM);
      tft.drawString("!ALERT!", tft.width() / 2 - 75, 30); // Adjust the position as needed
      //beep
      beepThreeTimes();

    }

  }
  else if (pageIndex == 24 )
  {
    delay(200) ;

  }
  else if (pageIndex == 25 && millis() - lastAlertInterval > 25)
  {

  }
}

//====================================================================================
//                                    Setup
//====================================================================================
void setup()
{
  silenceBuzzer();
  pinMode(buttonPin, INPUT);
  Serial.begin(115200);
  Serial.println("\n\n CryptoClock V4.1.4\n\n");
  currentIndex = 0 ;
  initSDCard();

  if (!LittleFS.begin()) {
    Serial.println("Failed to mount LittleFS!");
    return;
  }

  check_and_create_all();



  // Initialise the TFT
  tft.begin();
  if (rotate180 == true)
  {
    tft.setRotation(3);
  }
  else
  {
    tft.setRotation(1);
  }
  tft.fillScreen(TFT_BLACK);
  tft.setSwapBytes(true);
  tft.setTextSize(2);
  Serial.println("Start checkInversion");
  //check invasion
  checkInversion(tft); // เรียกแค่คำสั่งเดียวพอ!


  if (inversion)
  {
    tft.writecommand(ILI9341_GAMMASET); //Gamma curve selected
    tft.writedata(2);
    delay(120);
    tft.writecommand(ILI9341_GAMMASET); //Gamma curve selected
    tft.writedata(1);
    tft.writecommand(0x21); // Inversion ON
  }
  else
  {
    tft.writecommand(0x20); // Inversion OFF
  }



  ts.begin();

  silenceBuzzer();
  pinMode(16, OUTPUT);
  digitalWrite(16, HIGH);
  pinMode(17, OUTPUT);
  digitalWrite(17, HIGH);
  pinMode(16, OUTPUT);
  digitalWrite(16, HIGH);
  delay(20);
  silenceBuzzer();
  gif.begin(BIG_ENDIAN_PIXELS);

  //  analogWrite(21, 0);
  //  drawSdJpeg("/" + firmwareVersion +"/system/image/logo200.jpg", 60, 20);
  //  for (int i = 0 ; i < 255 ; i++ )
  //  {
  //    analogWrite(21, i);
  //    delay(2);
  //  }
  //  delay(1000);
  //  for (int i = 0 ; i < 255 ; i++ )
  //  {
  //    analogWrite(21, 255 - i);
  //    delay(2);
  //  }
  tft.fillScreen(TFT_BLACK);
  delay(100) ;
  analogWrite(21, 255);
  delay(10);

  if (sdStatus == true)
  {
    // อ่านค่าเหรียญจากไฟล์ coincurrency.txt
    if (!LittleFS.exists("/config.json")) {
      Serial.println("No config.json found. Saving default config...");
      save_default_config(); // ใช้ saveConfig ผ่านฟังก์ชันนี้
      save_default_coin();
      save_default_alert();
      load_config();
      delay(100);

      delay(100);
      load_coin();
      delay(100);
      load_profile();
      delay(100);
      load_alert();

    } else {
      Serial.println("Config.json exists. Loading config...");
      load_config();
      delay(100);
      if (maxCoin != 4)
      {
        maxCoin = 4;
        save_config();
      }
      delay(100);
      load_coin();
      delay(100);
      load_profile();
      delay(100);
      load_alert();
      String firmwareCurrentVersion = "4.1.4" ;
      if (firmwareVersion != firmwareCurrentVersion)
      {
        Serial.println("littlefs version match current version!");
        firmwareVersion = firmwareCurrentVersion ;
        deviceType = "ccn";
        deviceModel = "2025";
        save_config();
      }
      else
      {
        Serial.println("littlefs version not match current version!");
      }


    }

    loadAPIConfig();
    delay(500);

    adjustBrightness();
    delay(500);
    load_wifi();
    if (wifi_ssid == "" || wifi_password == "") { // ถ้าไม่มีข้อมูลให้เข้า AP Mode
      Serial.println("No WiFi config found. Entering AP Mode...");
      int n = WiFi.scanNetworks();
      jsonWiFi = "[";

      if (n == 0) {
        Serial.println("No networks found");
        jsonWiFi += "{\"error\":\"No networks found\"}";
      } else {
        Serial.print(n);
        Serial.println(" networks found");
        for (int i = 0; i < n; ++i) {
          if (i > 0) jsonWiFi += ",";
          jsonWiFi += "{\"ssid\":\"" + WiFi.SSID(i) + "\", \"frequency\":" + String(WiFi.channel(i)) + "}";
        }
      }

      jsonWiFi += "]";
      setupWiFiAPMode();
      animateAndCount(20);
      tft.fillScreen(TFT_BLACK);
      tft.setTextSize(2);                  // กำหนดขนาดข้อความ
      tft.setTextColor(TFT_WHITE);         // กำหนดสีข้อความ
      tft.drawString("Connect WiFi: CryptoClock", 5, 0, 1); // วาดข้อความบนหน้าจอ
      tft.setTextSize(1);
      tft.drawString("Goto:192.168.4.1 or Scan", 5, 40, 1);
      drawQRCode("http://192.168.4.1", 4 , 5 , 30);
    } else {
      WiFi.begin(wifi_ssid.c_str(), wifi_password.c_str());
      Serial.print("Connecting to WiFi");

      // Attempt to connect to WiFi network:
      tft.setTextSize(2);                  // Set text size
      tft.setTextColor(TFT_WHITE); // Set text color and background color

      // ปรับตำแหน่งของข้อความกลางจอ
      int textWidth = tft.textWidth("Connecting to WiFi");
      tft.drawString("Connecting to WiFi", TFT_WIDTH / 2 - textWidth / 2, 10, 2);
      int attempts = 0;
      while (WiFi.status() != WL_CONNECTED && attempts < 40) {
        delay(500);
        Serial.print(".");
        attempts++;

        // ลบหน้าจอและวาดแถบโหลดใหม่
        //tft.fillScreen(TFT_BLACK);
        int barWidth = map(attempts, 0, 20, 0, 240);
        tft.drawRoundRect(40, TFT_HEIGHT / 2 - 50, TFT_WIDTH, 20, 5, TFT_WHITE);  // วาดกรอบโค้ง

        if (barWidth > 5) {  // ตรวจสอบว่าความกว้างมากกว่ารัศมีโค้ง
          tft.fillRoundRect(40, TFT_HEIGHT / 2 - 50, barWidth, 20, 5, TFT_WHITE);  // วาดสี่เหลี่ยมโค้ง

          if (barWidth > 15) {
            tft.fillRect(55, TFT_HEIGHT / 2 - 50, barWidth - 15, 20, TFT_WHITE);  // วาดสี่เหลี่ยมธรรมดาทับ
          }
        }
        //tft.drawString(String(attempts) + " attempts", 10, 50, 2);
      }
      if (attempts < 20)
      {
        delay(100);
        attempts++;
        // ลบหน้าจอและวาดแถบโหลดใหม่
        //tft.fillScreen(TFT_BLACK);
        int barWidth = map(attempts, 0, 20, 0, 240);
        tft.drawRoundRect(40, TFT_HEIGHT / 2 - 50, TFT_WIDTH, 20, 5, TFT_WHITE);  // วาดกรอบโค้ง

        if (barWidth > 5) {  // ตรวจสอบว่าความกว้างมากกว่ารัศมีโค้ง
          tft.fillRoundRect(40, TFT_HEIGHT / 2 - 50, barWidth, 20, 5, TFT_WHITE);  // วาดสี่เหลี่ยมโค้ง

          if (barWidth > 15) {
            tft.fillRect(55, TFT_HEIGHT / 2 - 50, barWidth - 15, 20, TFT_WHITE);  // วาดสี่เหลี่ยมธรรมดาทับ
          }
        }
      }

      if (WiFi.status() != WL_CONNECTED) {
        Serial.println("Failed to connect. Entering AP Mode...");
        tft.fillScreen(TFT_BLACK);  // Clear the screen
        WiFi.mode(WIFI_STA);
        WiFi.disconnect();
        delay(100);
        int n = WiFi.scanNetworks();
        jsonWiFi = "[";

        if (n == 0) {
          Serial.println("No networks found");
          jsonWiFi += "{\"error\":\"No networks found\"}";
        } else {
          Serial.print(n);
          Serial.println(" networks found");
          for (int i = 0; i < n; ++i) {
            if (i > 0) jsonWiFi += ",";
            jsonWiFi += "{\"ssid\":\"" + WiFi.SSID(i) + "\", \"frequency\":" + String(WiFi.channel(i)) + "}";
          }
        }

        jsonWiFi += "]";
        Serial.println(jsonWiFi);
        setupWiFiAPMode();
        animateAndCount(5);
        tft.fillScreen(TFT_BLACK);
        tft.setTextSize(2);                  // กำหนดขนาดข้อความ
        tft.setTextColor(TFT_WHITE);         // กำหนดสีข้อความ
        tft.drawString("Connect WiFi: CryptoClock", 5, 0, 1); // วาดข้อความบนหน้าจอ
        tft.setTextSize(1);
        tft.drawString("Goto:192.168.4.1 or Scan", 5, 40, 1);

        drawQRCode("http://192.168.4.1", 4 , 5 , 30);


      } else { //ฟังชั่นหลัก ไวไฟเชื่อมต่อ
        setupTime();
        delay(100);
        Serial.println("Connected to WiFi");
        // ทำงานต่อไปหากเชื่อมต่อ WiFi สำเร็จ
        ipAddress = WiFi.localIP().toString();
        Serial.println(ipAddress);

        //check sd file
        //testSingleDownload();
        //createRootDirs();
        //verifyFiles();

        delay(10);
        downloadAllCoinIcons(false);
        delay(10);


        tft.fillScreen(TFT_BLACK);
        tft.setTextSize(2);                  // กำหนดขนาดข้อความ
        tft.setTextColor(TFT_WHITE);         // กำหนดสีข้อความ
        tft.drawString("IP: " + ipAddress, 10, 10, 2);
        tft.drawString("Scan for Manual", 10, 45, 1);
        String path = "/" + firmwareVersion + "/system/image/Manual.jpg";
        drawSdJpeg(path.c_str(), 96, 80);
        // drawSdJpeg("/" + firmwareVersion + "/system/image/Manual.jpg", 96, 80);

        newurl2 = "http://" + ipAddress  ;
        newurl = "Goto:" + ipAddress + " or Scan" ;
        delay(30);

        if (deviceId == "CCN000000" || deviceId == "0" || deviceId.isEmpty())
        {
          // Keep a stable, unique ID without registering with Google Scripts.
          String localId = WiFi.macAddress();
          localId.replace(":", "");
          deviceId = "CCN" + localId;
          save_config();
          Serial.println("Local device ID: " + deviceId);

        }
        else
        {
          Serial.print("DeviceId :");
          Serial.println(deviceId);
        }

        // No remote Google permission check; all original local features run.
        permission = true;
        // Use the permission variable as needed
        if (permission) {
          Serial.println("Permission granted");
          // Do something with the permission granted
        } else {
          Serial.println("Permission denied");
          // Do something with the permission denied
        }

        // Reset the checkComplete flag for the next iteration
        checkComplete = false;

        //permission = true;
        Serial.print("pageIndex:");
        Serial.println(pageIndex);
        if (permission == true)
        {
          if (permission && adsValue.toInt() > currentAds.toInt()) {
            Serial.println("Download Ads");
            String imageUrl = "https://raw.githubusercontent.com/bigboxthailand/cryptoclock/main/Ads/" + adsValue + ".jpg";
            downloadImage("ads" , imageUrl.c_str());
            String path = "/" + firmwareVersion + "/system/image/ads.jpg";
            drawSdJpeg(path.c_str(), 0, 0);
            currentAds = adsValue ;
            save_config() ;
            delay(1000);
          }
          USDTHBRate = fetchExchangeRate(currency[0], currency[1]);
          updateExchangeRate();
          delay(10);

          //auth
//          if (LittleFS.exists("/locked.flag")) {
//            Serial.println("Lock Found");
//            delay(300);
//
//            return;
//          }


          //pageIndex = 24; //test settings
          Serial.print("pageIndex: ");
          Serial.println(pageIndex);
          delay(10);
          delay(10);
          if (pageIndex == 1)
          {
            showPage1();
          }
          else if (pageIndex == 2)
          {
            if (coin[coinIndex] == "GOLD")
            {
              displayGoldOnScreen();
              startPriceTask();
            }
            else
            {
              displayBackgroundImageForPage2();
              showLogoAtTopMiddle();
              showLogoAtBottomMiddle();
              displayCoinOnScreen();
              displayPriceOnScreen(price[coinIndex]);
              displayHighLowPrice();
              startPriceTask();
            }
          }
          else if (pageIndex == 24)
          {
            Serial.println("Test Page24");

            showPage24();
          }
          else if (pageIndex == 25)
          {
            Serial.println("Test Page25");

            //checkInversion(tft);
          }
          else if (pageIndex == 26)
          {
            Serial.println("Show Page26 CDC");
            stopPriceTask();
            showPage26();
            lastUpdateTime = millis();
            pageChangeTime = millis();
          }
          esp_task_wdt_init(60, true); // Enable panic so ESP32 restarts
          esp_task_wdt_add(NULL); // Add current thread to WDT watch

          if (!SD.begin(5)) {
            Serial.println(F("SD.begin failed!"));
            //while (1) delay(0);
            sdStatus  = false ;
          }
          else
          {
            Serial.println(F("SD.begin OK!"));
            sdStatus = true;
            fileCount = countFiles(); // นับจำนวนไฟล์ที่อยู่ใน SD card
            Serial.print("fileCount:");
            Serial.println(fileCount);
          }


          if (pageIndex == 9 )
          {
            //delay(200) ;
            menuId = 9 ;
            showPage9();
          }

          checkInversion(tft);
          Serial.println("\n\n Fix Bitkub\n\n");
          Serial.println("\r\nInitialisation done.");
          lastUpdateTime = millis();
          pageChangeTime = millis();
          durationTask = millis();
          //  testDisplayPriceOnScreen();
        }
      }
    }
  }
  else
  {
  }
}





//====================================================================================
//                                    Loop
//====================================================================================

void loop() {

  // check permission
  if (sdStatus == false)
  {
    stopPriceTask();
    tft.fillScreen(TFT_BLACK);
    delay(100);
    tft.setTextSize(2);                  // กำหนดขนาดข้อความ
    tft.setTextColor(TFT_WHITE);         // กำหนดสีข้อความ
    tft.drawString("SD Card Problem", 10, 100, 2);
    delay(5000);
    ESP.restart();
  }
  if (permission == false )
  {
    if (WiFi.getMode() & WIFI_MODE_AP) {

      server.handleClient();
    }
    else
    {
      stopPriceTask();
      Serial.println("Pls Contact Admin");
      tft.fillScreen(TFT_BLACK);
      delay(1000);
      tft.setTextSize(2);                  // กำหนดขนาดข้อความ
      tft.setTextColor(TFT_WHITE);         // กำหนดสีข้อความ
      tft.drawString("Pls Contact Admin", 10, 100, 2);
      delay(1000);
      esp_task_wdt_reset();
    }

  }
  else if (permission == true)
  {
    //Serial.println("Permission Grant!");
    // "Kick" or "Feed" WDT
    esp_task_wdt_reset();
    if (WiFi.status() != WL_CONNECTED && !(WiFi.getMode() & WIFI_MODE_AP)) {
      Serial.println("WiFi disconnected. Attempting to reconnect...");
      WiFi.begin(wifi_ssid.c_str(), wifi_ssid.c_str());
      int attempts = 0;
      while (WiFi.status() != WL_CONNECTED && attempts < 10) { // ลองเชื่อมต่ออีกครั้งสูงสุด 10 ครั้ง
        delay(1000);
        Serial.print(".");
        attempts++;
      }
      if (WiFi.status() == WL_CONNECTED) {
        Serial.println("Reconnected to WiFi.");
        ipAddress = WiFi.localIP().toString();
        newurl2 = "http://" + ipAddress  ;
        newurl = "Goto:" + ipAddress + " or Scan" ;
      } else {
        Serial.println("Failed to reconnect. Entering AP Mode...");
        ESP.restart();
      }
    }
    // สำหรับการจัดการเว็บเซิร์ฟเวอร์ใน AP Mode
    if (WiFi.getMode() & WIFI_MODE_AP) {
      server.handleClient();
      // Check for timeout
      if (pageWiFiStartTime == 0) { // If starting timer for page 4, record the time
        pageWiFiStartTime = millis();
      }
      if (millis() - pageWiFiStartTime > pageWiFiTimeout) {
        Serial.println("Timeout on page 5, restarting...");
        pageWiFiStartTime = 0;
        ESP.restart(); // Restart the ESP32

      }

    }
    else //wifi connect
    {
      struct tm timeinfo;
      if (!getLocalTime(&timeinfo)) {
        Serial.println("Failed to obtain time");
        delay(1000);
        return;
      }

      // แสดงเวลา
      int currentHour = timeinfo.tm_hour;
      int currentMinute = timeinfo.tm_min;
      int currentDay = timeinfo.tm_wday;
      // ตรวจสอบเวลา (ทุกวันจันทร์ เวลา 02:00 - 02:30)
      //if (currentDay == 1 && currentHour == 2 && currentMinute < 30) {




      if (digitalRead(buttonPin) == LOW) {
        delay(50); // Debounce delay
        if (digitalRead(buttonPin) == LOW) {
          //run server
          stopPriceTask();
          pageIndex = 5;
          Serial.print("page : ");
          Serial.println(pageIndex);
        }
      }

      if (Serial.available()) {
        String input = Serial.readStringUntil('\n');
        input.trim(); // ตัดช่องว่าง/Enter ด้านท้าย

      }
      if (waitforclick == true && millis() - updateClickInterval >= 3000 && pageIndex == 2)
      {
        waitforclick = false ;
        updateExchangeRate();
        //save coinindex to coincurrency
        save_coin() ;
        delay(50);

        Serial.println("start task");
        startPriceTask();
        delay(50);
      }

      if (waitforprice == false && pageIndex != 8)
      {
        TouchPoint touch = checkTouchScreen();
        handleTouchInput(touch); // Pass the touch point to the handler
      }
      if (millis() - alertTime >= 300000 && pauseAlert == true && alertEnable == true) //reset snooze
      {

        pauseAlert = false ;
      }

      // ยังคงอัปเดตเวลาหรือการแสดงผลอื่นๆที่จำเป็น
      displaypageIndex();
    }
  }
}
