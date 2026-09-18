#include "BootstrapAssets.h"

#include <HTTPClient.h>
#include <SD.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <mbedtls/sha256.h>

namespace {
const char ASSET_BASE_URL[] = "https://raw.githubusercontent.com/cryptoclocks/BitClock-Workshop-Student/main/sdcard/bootstrap";
const size_t MAX_BOOTSTRAP_ASSET_BYTES = 256UL * 1024UL;

struct BootstrapAsset {
  const char* remotePath;
  const char* localPath;
  const char* sha256;
};

const BootstrapAsset BOOTSTRAP_ASSETS[] = {
  {"profile.jpg", "/profile.jpg", "2f52440aacfd8eedf800e098b9b4e693c36205f9f56b0d1cb345cb7a1cd4dc53"},
  {"slides/slide1.jpg", "/slides/slide1.jpg", "b58be4337bf997a255c403e8d95fb9aa3719bec9d24a747e7a329339ab1c65a8"},
};

void ensureSlideDirectory() {
  if (!SD.exists("/slides")) SD.mkdir("/slides");
}

String digestToHex(const uint8_t digest[32]) {
  const char hex[] = "0123456789abcdef";
  char text[65];
  for (size_t i = 0; i < 32; i++) {
    text[i * 2] = hex[digest[i] >> 4];
    text[i * 2 + 1] = hex[digest[i] & 0x0F];
  }
  text[64] = '\0';
  return String(text);
}

bool downloadOneAsset(const BootstrapAsset& asset, String& error) {
  if (SD.exists(asset.localPath)) return true;  // ไม่เขียนทับรูปที่ผู้เรียนอัปโหลดเอง
  if (String(asset.localPath).startsWith("/slides/")) ensureSlideDirectory();

  String url = String(ASSET_BASE_URL) + "/" + asset.remotePath;
  String tempPath = String(asset.localPath) + ".part";
  if (SD.exists(tempPath)) SD.remove(tempPath);

  WiFiClientSecure client;
  client.setInsecure();  // SHA-256 ด้านล่างยืนยันว่าเนื้อหาเป็น Asset ที่กำหนดไว้
  HTTPClient http;
  http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
  http.setTimeout(15000);
  if (!http.begin(client, url)) {
    error = "เปิดการเชื่อมต่อ GitHub ไม่ได้";
    return false;
  }

  int code = http.GET();
  if (code != HTTP_CODE_OK) {
    error = "GitHub HTTP " + String(code);
    http.end();
    return false;
  }

  int length = http.getSize();
  if (length > static_cast<int>(MAX_BOOTSTRAP_ASSET_BYTES)) {
    error = "ไฟล์ใหญ่เกินกำหนด";
    http.end();
    return false;
  }

  File output = SD.open(tempPath, FILE_WRITE);
  if (!output) {
    error = "สร้างไฟล์บน SD ไม่ได้";
    http.end();
    return false;
  }

  mbedtls_sha256_context sha;
  mbedtls_sha256_init(&sha);
  mbedtls_sha256_starts_ret(&sha, 0);
  WiFiClient* stream = http.getStreamPtr();
  uint8_t buffer[512];
  size_t written = 0;
  uint32_t lastDataAt = millis();

  while (http.connected() || stream->available()) {
    size_t available = stream->available();
    if (available == 0) {
      if (millis() - lastDataAt > 15000) {
        error = "ดาวน์โหลดหมดเวลา";
        break;
      }
      delay(1);
      continue;
    }
    lastDataAt = millis();
    size_t readCount = stream->readBytes(buffer, min(available, sizeof(buffer)));
    if (readCount == 0) continue;
    written += readCount;
    if (written > MAX_BOOTSTRAP_ASSET_BYTES || output.write(buffer, readCount) != readCount) {
      error = "เขียนไฟล์ไม่สำเร็จ";
      break;
    }
    mbedtls_sha256_update_ret(&sha, buffer, readCount);
  }

  uint8_t digest[32];
  mbedtls_sha256_finish_ret(&sha, digest);
  mbedtls_sha256_free(&sha);
  output.close();
  http.end();

  bool completed = error.isEmpty() && written > 0 && (length < 0 || written == static_cast<size_t>(length));
  if (!completed) {
    if (error.isEmpty()) error = "ขนาดไฟล์ไม่ครบ";
    SD.remove(tempPath);
    return false;
  }
  if (digestToHex(digest) != asset.sha256) {
    error = "checksum ไม่ตรง";
    SD.remove(tempPath);
    return false;
  }
  if (!SD.rename(tempPath, asset.localPath)) {
    error = "ย้ายไฟล์บน SD ไม่ได้";
    SD.remove(tempPath);
    return false;
  }
  return true;
}
}  // namespace

bool downloadMissingBootstrapAssets(String& summary) {
  summary = "";
  if (WiFi.status() != WL_CONNECTED) {
    summary = "ยังไม่ได้เชื่อม Wi-Fi";
    return false;
  }

  uint8_t downloaded = 0;
  uint8_t alreadyPresent = 0;
  for (const BootstrapAsset& asset : BOOTSTRAP_ASSETS) {
    if (SD.exists(asset.localPath)) {
      alreadyPresent++;
      continue;
    }
    String error;
    Serial.printf("Bootstrap asset: %s\n", asset.remotePath);
    if (!downloadOneAsset(asset, error)) {
      summary = String("ดาวน์โหลด ") + asset.remotePath + " ไม่สำเร็จ: " + error;
      Serial.println(summary);
      return false;
    }
    downloaded++;
  }

  summary = downloaded == 0
    ? "Asset เริ่มต้นมีครบแล้ว"
    : "ดาวน์โหลด Asset เริ่มต้น " + String(downloaded) + " ไฟล์สำเร็จ";
  if (alreadyPresent > 0 && downloaded > 0) summary += " (ข้ามไฟล์เดิม " + String(alreadyPresent) + " ไฟล์)";
  Serial.println(summary);
  return true;
}
