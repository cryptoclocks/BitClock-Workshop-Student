#include "cc_sd.h"
#include <SD.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "GlobalModule.h"
#include "GlobalParameter.h"
#define SD_CS 5




// สร้างโฟลเดอร์ย่อยตาม path เช่น "website/index.html"
void createFoldersIfNeeded(String path) {
  int start = 0;
  while (true) {
    int slashIndex = path.indexOf('/', start + 1);
    if (slashIndex == -1) break;
    String folder = path.substring(0, slashIndex);
    if (!SD.exists(folder)) {
      SD.mkdir(folder.c_str());
    }
    start = slashIndex;
  }
}

static WiFiClientSecure secureClient;


// ดาวน์โหลดและบันทึกไฟล์ จาก url → savePath
bool downloadFile(String url, String savePath) {
  http3.end();
  // สร้างโฟลเดอร์ย่อย (ถ้ายังไม่มี)
  createFoldersIfNeeded(savePath);

  Serial.printf("📥 ดาวน์โหลด: %s → %s\n", url.c_str(), savePath.c_str());
  secureClient.setInsecure(); // ถ้าต้องการ security เต็มให้เปลี่ยนเป็น setCACert(...)

  // อุ่น DNS หน่อย กันฟาวล์บางบอร์ด
  IPAddress ip;
  WiFi.hostByName("raw.githubusercontent.com", ip);

  // เริ่ม HTTP (ผูกกับ secure client + เปิด redirect + ตั้ง timeout)
  http3.end();
  if (!http3.begin(secureClient, url)) {
    Serial.println("❌ http.begin() ล้มเหลว");
    return false;
  }
  http3.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
  http3.setUserAgent("CryptoClock/4.1.4");
  http3.setTimeout(15000);

  Serial.printf("📥 ดาวน์โหลด: %s → %s\n", url.c_str(), savePath.c_str());
  int httpCode = http3.GET();
  if (httpCode != HTTP_CODE_OK) {
    Serial.printf("❌ HTTP Error %d\n", httpCode);
    http3.end();
    return false;
  }

  int contentLength = http3.getSize();
  WiFiClient* stream = http3.getStreamPtr();

  File file = SD.open(savePath.c_str(), FILE_WRITE);
  if (!file) {
    Serial.println("❌ เปิดไฟล์ไม่สำเร็จ");
    http3.end();
    return false;
  }

  uint8_t buff[2048];
  int written = 0;
  uint32_t lastTick = millis();
  while (http3.connected() && (contentLength > 0 || contentLength == -1)) {
    size_t avail = stream->available();
    if (avail) {
      int toRead = avail > sizeof(buff) ? sizeof(buff) : avail;
      int got = stream->readBytes(buff, toRead);
      if (got > 0) {
        file.write(buff, got);
        written += got;
        if (contentLength > 0) contentLength -= got;
        lastTick = millis();
      }
    } else {
      // กันค้างถ้าเน็ตช้า
      if (millis() - lastTick > 15000) { Serial.println("⏱ stream timeout"); break; }
      delay(1);
    }
  }
  file.close();
  http3.end();

    Serial.printf("✅ บันทึกแล้ว %u ไบต์\n", (unsigned)written);
  if (written == 0) {
    SD.remove(savePath);
    return false;
  }
  return true;
}


void testSingleDownload() {
  String url = "https://raw.githubusercontent.com/cryptoclocks/2025/main/sd/4.1.4/logo.png";
  String savePath = "/4.1.4/logo.png";
  if (downloadFile(url, savePath)) {
    Serial.println("✅ OK");
  } else {
    Serial.println("❌ FAIL");
  }
}
