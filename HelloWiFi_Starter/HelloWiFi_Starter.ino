/*
  HelloWiFi Starter
  Board: ESP32-2432S028R

  บทเรียนแรก: ตั้งชื่อ Wi-Fi, เชื่อมต่อ, พิมพ์ข้อความลง Serial Monitor
  และวาด Hello World ตรงกลางจอ 320 x 240 px
*/

#include <WiFi.h>
#include <LittleFS.h>
#include <TFT_eSPI.h>
#include <XPT2046_Bitbang.h>

// แก้เฉพาะ 2 บรรทัดนี้ก่อน Upload
const char* WIFI_SSID = "YOUR_WIFI_NAME";
const char* WIFI_PASSWORD = "YOUR_WIFI_PASSWORD";

// Pin ของ Touch บน ESP32-2432S028R
constexpr uint8_t TOUCH_MOSI = 32;
constexpr uint8_t TOUCH_MISO = 39;
constexpr uint8_t TOUCH_CLK  = 25;
constexpr uint8_t TOUCH_CS   = 33;
constexpr uint16_t TOUCH_THRESHOLD = 280;
const char* DISPLAY_MODE_FILE = "/hello-display-mode.txt";

TFT_eSPI tft = TFT_eSPI();
XPT2046_Bitbang touch(TOUCH_MOSI, TOUCH_MISO, TOUCH_CLK, TOUCH_CS);

bool chooseDisplayMode = false;
bool touchWasDown = false;
TouchPoint lastTouch = {0, 0, 0, 0, 0};

void applyDisplayMode(bool inversionOn) {
  if (inversionOn) {
    tft.writecommand(ILI9341_GAMMASET);
    tft.writedata(2);
    delay(120);
    tft.writecommand(ILI9341_GAMMASET);
    tft.writedata(1);
    tft.writecommand(0x21); // Display inversion ON: Black mode on this board
  } else {
    tft.writecommand(0x20); // Display inversion OFF: White mode
  }
}

bool loadDisplayMode(bool& inversionOn) {
  if (!LittleFS.exists(DISPLAY_MODE_FILE)) return false;
  File file = LittleFS.open(DISPLAY_MODE_FILE, "r");
  if (!file) return false;
  String value = file.readStringUntil('\n');
  file.close();
  value.trim();
  if (value != "black" && value != "white") return false;
  inversionOn = (value == "black");
  return true;
}

void saveDisplayMode(bool inversionOn) {
  File file = LittleFS.open(DISPLAY_MODE_FILE, "w");
  if (!file) return;
  file.println(inversionOn ? "black" : "white");
  file.close();
}

void drawDisplayModeSetup() {
  tft.writecommand(0x20); // Make the setup screen readable before a mode is chosen.
  tft.fillScreen(TFT_BLACK);
  tft.setTextDatum(MC_DATUM);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.drawString("Setup display", 160, 38, 2);
  tft.drawString("Choose screen color", 160, 68, 2);

  tft.fillRoundRect(24, 120, 125, 68, 10, TFT_DARKGREY);
  tft.setTextColor(TFT_WHITE, TFT_DARKGREY);
  tft.drawString("Black", 86, 143, 4);

  tft.fillRoundRect(171, 120, 125, 68, 10, TFT_WHITE);
  tft.setTextColor(TFT_BLACK, TFT_WHITE);
  tft.drawString("White", 233, 143, 4);

  tft.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
  tft.drawString("Tap once. Board will restart.", 160, 216, 2);
}

void handleDisplayModeTouch() {
  TouchPoint point = touch.getTouch();
  if (point.zRaw > TOUCH_THRESHOLD) {
    touchWasDown = true;
    lastTouch = point;
    return;
  }
  if (!touchWasDown) return;
  touchWasDown = false;

  if (lastTouch.y < 120 || lastTouch.y > 188) return;
  if (lastTouch.x >= 24 && lastTouch.x <= 149) {
    saveDisplayMode(true);
  } else if (lastTouch.x >= 171 && lastTouch.x <= 296) {
    saveDisplayMode(false);
  } else {
    return;
  }
  ESP.restart();
}

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

  if (!LittleFS.begin(true)) {
    showMessage("Hello World", "LittleFS error", TFT_RED);
    return;
  }
  touch.begin();

  bool inversionOn = false;
  if (!loadDisplayMode(inversionOn)) {
    chooseDisplayMode = true;
    drawDisplayModeSetup();
    return;
  }
  applyDisplayMode(inversionOn);
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
  if (chooseDisplayMode) {
    handleDisplayModeTouch();
  }
}
