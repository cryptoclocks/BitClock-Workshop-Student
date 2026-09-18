/*
  HelloWiFi Starter
  Board: ESP32-2432S028R

  บทเรียนแรก: ตั้งชื่อ Wi-Fi, เชื่อมต่อ, พิมพ์ข้อความลง Serial Monitor
  และวาด Hello World ตรงกลางจอ 320 x 240 px
*/

#include <WiFi.h>
#include <TFT_eSPI.h>

// แก้เฉพาะ 2 บรรทัดนี้ก่อน Upload
const char* WIFI_SSID = "ใส่ชื่อ_WiFi_2.4GHz";
const char* WIFI_PASSWORD = "ใส่รหัสผ่าน_WiFi";

TFT_eSPI tft = TFT_eSPI();

void showMessage(const String& title, const String& detail, uint16_t color) {
  tft.fillScreen(TFT_BLACK);
  tft.setTextDatum(MC_DATUM);
  tft.setTextColor(color, TFT_BLACK);
  tft.drawString(title, tft.width() / 2, tft.height() / 2 - 14, 4);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.drawString(detail, tft.width() / 2, tft.height() / 2 + 24, 2);
}

void connectWiFi() {
  Serial.printf("Connecting to: %s\n", WIFI_SSID);
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  unsigned long startedAt = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - startedAt < 15000) {
    Serial.print('.');
    delay(500);
  }
  Serial.println();
}

void setup() {
  Serial.begin(115200);

  tft.begin();
  tft.setRotation(1);     // แนวนอน 320 x 240
  tft.setSwapBytes(true);
  showMessage("Hello World", "Starting Wi-Fi...", TFT_CYAN);

  connectWiFi();

  if (WiFi.status() == WL_CONNECTED) {
    String ip = WiFi.localIP().toString();
    Serial.println("Wi-Fi connected");
    Serial.println("IP address: " + ip);
    showMessage("Hello World", "Wi-Fi: " + ip, TFT_GREEN);
  } else {
    Serial.println("Wi-Fi connection failed");
    showMessage("Hello World", "Wi-Fi not connected", TFT_ORANGE);
  }
}

void loop() {
  // โค้ดเริ่มต้นยังไม่ต้องทำอะไรใน loop()
}
