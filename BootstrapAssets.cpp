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
  {"system/page2.jpg", "/system/page2.jpg", "7ad5cc2692d822e49c73d2669d7600e877f62fd59de3fec1ce9cda1b22894129"},
  {"system/btc.jpg", "/system/btc.jpg", "0bf01699270be8311319545172993175f99cc315bdb9a51e61235b41c3c14763"},
  {"system/kub.jpg", "/system/kub.jpg", "8218dfe223e63762d9e5ef8ff161563f807e75aceb499760d5b0cc8042315ed6"},
  {"system/doge.jpg", "/system/doge.jpg", "680ab1549586086454f9478666c29248e9e020d23d7015b0a6adba3087ed6fd4"},
  {"system/usdt.jpg", "/system/usdt.jpg", "912c85ac633c552d30d97f4a576bb18d9291c187d49bc24eb1f243960f6b450f"},
  {"system/logo.jpg", "/system/logo.jpg", "609550eeb0f6cd3672cc74bf0ccc77be8f6d85ea7cabbfd350ae50c33269dbb3"},
  {"slides/slide1.jpg", "/slides/slide1.jpg", "1dd6c395e96e5c21b6656e96d94e513b6a660481a86f1462059c15277847e9bc"},
  {"slides/slide2.jpg", "/slides/slide2.jpg", "fda2a6574a0d30f5e7af9fd56e0cf9573bb63261ca2139022e2ed1b2415f2274"},
  {"slides/slide3.jpg", "/slides/slide3.jpg", "d259eb95cd6f14a0aafd8d567c5202a370ebe67a617a6ea61279d525611550ef"},
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
  if (String(asset.localPath).startsWith("/system/") && !SD.exists("/system")) SD.mkdir("/system");

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
  Serial.printf("Bootstrap HTTP %d, bytes: %d\n", code, length);
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

  uint32_t startedAt = millis();
  while (http.connected() || stream->available()) {
    if (length >= 0 && written >= static_cast<size_t>(length)) break;
    if (millis() - startedAt > 20000) {
      error = "ดาวน์โหลดเกิน 20 วินาที";
      break;
    }
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
