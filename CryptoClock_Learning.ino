/*
  CryptoClock Learning Edition

  4 หน้าหลัก:
  1) Profile
  2) ราคาเหรียญจาก Bitkub API (วน 4 เหรียญ)
  3) CDC Action Zone จากกราฟ BTC/THB 4H
  4) ภาพสไลด์จาก SD Card

  หน้าต่อ Wi-Fi แสดงเฉพาะตอน WiFiManager เปิด Config Portal
  ไม่มี MQTT, OTA, Alarm, Auth, Payment หรือ Server ของระบบเดิม
*/
#include <Arduino.h>
#include <WiFi.h>
#include <WiFiManager.h>
#include <WebServer.h>
#include <Preferences.h>
#include <TFT_eSPI.h>
#include <SPI.h>
#include <SD.h>
#include <time.h>

#include "AppConfig.h"
#include "CoinData.h"
#include "BitkubApi.h"
#include "BootstrapAssets.h"
#include "Pages.h"

enum MainPage {
  PAGE_PROFILE,
  PAGE_COIN,
  PAGE_CDC,
  PAGE_SD_SLIDE
};

TFT_eSPI tft;
WiFiManager wifiManager;
WebServer settingsServer(80);
Preferences preferences;
CdcData cdcData = {};
SPIClass sdSpi(HSPI);  // แยกบัส SD ออกจากบัส TFT ของบอร์ด ESP32-2432S028R

MainPage currentPage = PAGE_PROFILE;
size_t currentCoin = 0;
uint8_t currentSdSlide = 1;
uint8_t sdSlideCount = 0;
bool sdReady = false;
bool pageEnabled[] = {true, true, true, true};
unsigned long pageDurationMs[] = {PROFILE_PAGE_MS, COIN_PAGE_MS, CDC_PAGE_MS, SD_SLIDE_MS};
File uploadFile;
String bootstrapStatus;

unsigned long pageStartedAt = 0;
unsigned long lastPriceUpdateAt = 0;
unsigned long lastCdcUpdateAt = 0;
unsigned long lastReconnectAttemptAt = 0;

uint8_t countSdSlides();
void enterPage(MainPage page);

void showBootstrapStatus(const String& message, uint16_t color) {
  tft.fillScreen(TFT_BLACK);
  tft.setTextDatum(MC_DATUM);
  tft.setTextColor(TFT_CYAN, TFT_BLACK);
  tft.drawString("Preparing SD Card...", tft.width() / 2, 92, 2);
  tft.setTextColor(color, TFT_BLACK);
  tft.drawString(message, tft.width() / 2, 126, 2);
  tft.setTextDatum(TL_DATUM);
}

void bootstrapMissingAssets() {
  if (!sdReady) {
    bootstrapStatus = "ไม่พบ SD Card จึงยังดาวน์โหลด Asset ไม่ได้";
    return;
  }
  showBootstrapStatus("Checking starter assets", TFT_LIGHTGREY);
  bool success = downloadMissingBootstrapAssets(bootstrapStatus);
  sdSlideCount = countSdSlides();
  showBootstrapStatus(success ? "Starter assets ready" : "Asset download failed", success ? TFT_GREEN : TFT_RED);
  delay(900);
}

void onWifiConfigMode(WiFiManager* manager) {
  showWifiSetupPage(tft, manager->getConfigPortalSSID());
}

void loadSettings() {
  preferences.begin("clockcfg", true);
  for (size_t i = 0; i < COIN_COUNT; i++) {
    String symbol = preferences.getString(("coin" + String(i)).c_str(), coins[i].symbol);
    setCoinAt(i, symbol);
  }
  for (size_t i = 0; i < 4; i++) {
    pageEnabled[i] = preferences.getBool(("page" + String(i)).c_str(), true);
    uint32_t seconds = preferences.getUInt(("duration" + String(i)).c_str(), pageDurationMs[i] / 1000UL);
    pageDurationMs[i] = constrain(seconds, 3UL, 60UL) * 1000UL;
  }
  preferences.end();
}

void saveSettings() {
  preferences.begin("clockcfg", false);
  for (size_t i = 0; i < COIN_COUNT; i++) preferences.putString(("coin" + String(i)).c_str(), coins[i].symbol);
  for (size_t i = 0; i < 4; i++) {
    preferences.putBool(("page" + String(i)).c_str(), pageEnabled[i]);
    preferences.putUInt(("duration" + String(i)).c_str(), pageDurationMs[i] / 1000UL);
  }
  preferences.end();
}

String selectedOption(const char* symbol) {
  String html;
  for (size_t i = 0; i < supportedCoinCount(); i++) {
    String value = supportedCoinSymbol(i);
    html += "<option value='" + value + "'" + (value == symbol ? " selected" : "") + ">";
    html += value + " — " + supportedCoinName(i) + "</option>";
  }
  return html;
}

String settingCheckbox(size_t index, const char* label) {
  return "<label class='check'><input type='checkbox' name='page" + String(index) + "' " +
    (pageEnabled[index] ? "checked" : "") + "> " + label + "</label>";
}

String imageFileList() {
  if (!sdReady) return "<p>ไม่พบ SD Card</p>";
  String html = "<ul>";
  const char* profilePath = "/profile.jpg";
  if (SD.exists(profilePath)) html += "<li>profile.jpg <a href='/delete?file=profile'>ลบ</a></li>";
  for (uint8_t i = 1; i <= MAX_SD_SLIDES; i++) {
    String path = "/slides/slide" + String(i) + ".jpg";
    if (SD.exists(path)) html += "<li>slide" + String(i) + ".jpg <a href='/delete?file=slide" + String(i) + "'>ลบ</a></li>";
  }
  return html + "</ul>";
}

void handleSettingsHome() {
  String html = F("<!doctype html><html lang='th'><meta name='viewport' content='width=device-width,initial-scale=1'>"
    "<title>CryptoClock Settings</title><style>body{max-width:720px;margin:auto;padding:18px;font:16px system-ui;background:#071829;color:#eef8ff}"
    "section{background:#10293e;border:1px solid #28506b;border-radius:12px;padding:16px;margin:14px 0}h1{color:#35d7e8}h2{font-size:19px}label,select,input{display:block;margin:9px 0}select,input[type=number]{padding:8px;width:100%;box-sizing:border-box}"
    ".check{display:inline-block;margin-right:14px}.check input{display:inline;width:auto}button{background:#35d7e8;border:0;border-radius:8px;padding:11px 16px;font-weight:bold}small{color:#b4c8d8}</style>"
    "<h1>CryptoClock Settings</h1><p>IP: ");
  html += WiFi.localIP().toString();
  html += F(" · ไม่มีฟีเจอร์แจ้งเตือน</p><form action='/save' method='post'><section><h2>เหรียญที่แสดง</h2><small>เลือกได้ 4 ช่อง ข้อมูลราคาอ่านจาก Bitkub</small>");
  for (size_t i = 0; i < COIN_COUNT; i++) {
    html += "<label>ช่อง " + String(i + 1) + "<select name='coin" + String(i) + "'>" + selectedOption(coins[i].symbol) + "</select></label>";
  }
  html += F("</section><section><h2>หน้าที่ให้วนแสดง</h2>");
  html += settingCheckbox(PAGE_PROFILE, "Profile");
  html += settingCheckbox(PAGE_COIN, "ราคาเหรียญ");
  html += settingCheckbox(PAGE_CDC, "CDC Action Zone");
  html += settingCheckbox(PAGE_SD_SLIDE, "SD Slide");
  html += F("<h2>เวลาแสดงต่อหน้า (วินาที)</h2>");
  const char* durationLabels[] = {"Profile", "เหรียญแต่ละรายการ", "CDC", "รูป SD แต่ละภาพ"};
  for (size_t i = 0; i < 4; i++) {
    html += "<label>" + String(durationLabels[i]) + "<input type='number' min='3' max='60' name='duration" + String(i) + "' value='" + String(pageDurationMs[i] / 1000UL) + "'></label>";
  }
  html += F("<button type='submit'>บันทึกการตั้งค่า</button></section></form>"
    "<section><h2>อัปโหลดรูปลง SD Card</h2><small>Profile: JPEG 96×96 · Slide: JPEG 320×240</small>"
    "<form action='/upload' method='post' enctype='multipart/form-data'><label>ตำแหน่งรูป<select name='target'>"
    "<option value='profile'>Profile (/profile.jpg)</option><option value='slide1'>Slide 1</option><option value='slide2'>Slide 2</option><option value='slide3'>Slide 3</option><option value='slide4'>Slide 4</option><option value='slide5'>Slide 5</option><option value='slide6'>Slide 6</option><option value='slide7'>Slide 7</option><option value='slide8'>Slide 8</option></select></label>"
    "<input type='file' name='image' accept='.jpg,.jpeg,image/jpeg' required><button type='submit'>อัปโหลด JPEG</button></form><h2>รูปที่มีอยู่</h2>");
  html += imageFileList();
  html += F("</section>"
    "<section><h2>Asset เริ่มต้นจาก GitHub</h2><small>ดาวน์โหลดเฉพาะไฟล์ที่ยังไม่มี: profile.jpg และ slide1.jpg · ไฟล์ที่อัปโหลดเองจะไม่ถูกเขียนทับ</small>"
    "<form action='/bootstrap-assets' method='post'><button type='submit'>ดาวน์โหลด Asset ที่ยังขาด</button></form><p>");
  html += bootstrapStatus;
  html += F("</p></section>"
    "<section><h2>สถานะ SD Card</h2><p>SD: ");
  html += sdReady ? "พร้อมใช้งาน" : "ไม่พบ SD Card";
  html += " · สไลด์: " + String(sdSlideCount) + " ภาพ</p></section></html>";
  settingsServer.send(200, "text/html; charset=utf-8", html);
}

void handleSettingsSave() {
  for (size_t i = 0; i < COIN_COUNT; i++) setCoinAt(i, settingsServer.arg("coin" + String(i)));
  bool hasPage = false;
  for (size_t i = 0; i < 4; i++) {
    pageEnabled[i] = settingsServer.hasArg("page" + String(i));
    hasPage |= pageEnabled[i];
    uint32_t seconds = settingsServer.arg("duration" + String(i)).toInt();
    pageDurationMs[i] = constrain(seconds, 3UL, 60UL) * 1000UL;
  }
  if (!hasPage) pageEnabled[PAGE_PROFILE] = true;
  saveSettings();
  fetchAllBitkubTickers();
  enterPage(PAGE_PROFILE);
  settingsServer.sendHeader("Location", "/", true);
  settingsServer.send(303);
}

String uploadTargetPath() {
  String target = settingsServer.arg("target");
  if (target == "profile") return "/profile.jpg";
  if (target.startsWith("slide")) {
    int number = target.substring(5).toInt();
    if (number >= 1 && number <= MAX_SD_SLIDES) return "/slides/slide" + String(number) + ".jpg";
  }
  return "";
}

void handleImageUpload() {
  HTTPUpload& upload = settingsServer.upload();
  if (!sdReady) return;
  if (upload.status == UPLOAD_FILE_START) {
    String path = uploadTargetPath();
    if (!path.isEmpty()) {
      if (SD.exists(path)) SD.remove(path);  // อัปโหลดซ้ำต้องแทนที่ไฟล์เดิม ไม่ใช่ต่อท้าย
      uploadFile = SD.open(path, FILE_WRITE);
    }
  } else if (upload.status == UPLOAD_FILE_WRITE && uploadFile) {
    uploadFile.write(upload.buf, upload.currentSize);
  } else if (upload.status == UPLOAD_FILE_END && uploadFile) {
    uploadFile.close();
  }
}

void finishImageUpload() {
  sdSlideCount = countSdSlides();
  if (currentPage == PAGE_PROFILE) showProfilePage(tft, sdReady);
  settingsServer.sendHeader("Location", "/", true);
  settingsServer.send(303);
}

void handleDeleteImage() {
  String path;
  String file = settingsServer.arg("file");
  if (file == "profile") path = "/profile.jpg";
  else if (file.startsWith("slide")) {
    int number = file.substring(5).toInt();
    if (number >= 1 && number <= MAX_SD_SLIDES) path = "/slides/slide" + String(number) + ".jpg";
  }
  if (!path.isEmpty() && SD.exists(path)) SD.remove(path);
  sdSlideCount = countSdSlides();
  settingsServer.sendHeader("Location", "/", true);
  settingsServer.send(303);
}

void handleBootstrapAssets() {
  bootstrapMissingAssets();
  settingsServer.sendHeader("Location", "/", true);
  settingsServer.send(303);
}

void setupSettingsServer() {
  settingsServer.on("/", HTTP_GET, handleSettingsHome);
  settingsServer.on("/save", HTTP_POST, handleSettingsSave);
  settingsServer.on("/upload", HTTP_POST, finishImageUpload, handleImageUpload);
  settingsServer.on("/delete", HTTP_GET, handleDeleteImage);
  settingsServer.on("/bootstrap-assets", HTTP_POST, handleBootstrapAssets);
  settingsServer.onNotFound([]() { settingsServer.send(404, "text/plain", "Not found"); });
  settingsServer.begin();
  Serial.printf("Settings: http://%s/\n", WiFi.localIP().toString().c_str());
}

uint8_t countSdSlides() {
  if (!sdReady) return 0;
  uint8_t count = 0;
  for (uint8_t i = 1; i <= MAX_SD_SLIDES; i++) {
    String path = "/slides/slide" + String(i) + ".jpg";
    if (!SD.exists(path)) break;
    count++;
  }
  return count;
}

void enterPage(MainPage page) {
  currentPage = page;
  pageStartedAt = millis();

  if (page == PAGE_PROFILE) {
    showProfilePage(tft, sdReady);
    return;
  }

  if (page == PAGE_COIN) {
    currentCoin = 0;
    showCoinPage(tft, coins[currentCoin], currentCoin);
    return;
  }

  if (page == PAGE_CDC) {
    tft.fillScreen(TFT_BLACK);
    tft.setTextDatum(MC_DATUM);
    tft.setTextColor(TFT_CYAN, TFT_BLACK);
    tft.drawString("Loading CDC Action Zone...", tft.width() / 2, tft.height() / 2, 2);

    if (!cdcData.hasData || millis() - lastCdcUpdateAt >= CDC_REFRESH_MS) {
      if (fetchBitkubCdcHistory(cdcData)) lastCdcUpdateAt = millis();
    }
    showCdcPage(tft, cdcData);
    pageStartedAt = millis();
    return;
  }

  currentSdSlide = 1;
  if (sdSlideCount == 0) showSdMissingPage(tft);
  else showSdSlidePage(tft, currentSdSlide, sdReady);
}

void enterNextEnabledPage() {
  for (uint8_t offset = 1; offset <= 4; offset++) {
    MainPage candidate = static_cast<MainPage>((currentPage + offset) % 4);
    if (pageEnabled[candidate]) {
      enterPage(candidate);
      return;
    }
  }
  enterPage(PAGE_PROFILE);
}

void updatePageSequence() {
  unsigned long elapsed = millis() - pageStartedAt;

  if (currentPage == PAGE_PROFILE && elapsed >= pageDurationMs[PAGE_PROFILE]) {
    enterNextEnabledPage();
  } else if (currentPage == PAGE_COIN && elapsed >= pageDurationMs[PAGE_COIN]) {
    if (currentCoin + 1 < COIN_COUNT) {
      currentCoin++;
      showCoinPage(tft, coins[currentCoin], currentCoin);
      pageStartedAt = millis();
    } else {
      enterNextEnabledPage();
    }
  } else if (currentPage == PAGE_CDC && elapsed >= pageDurationMs[PAGE_CDC]) {
    enterNextEnabledPage();
  } else if (currentPage == PAGE_SD_SLIDE && elapsed >= pageDurationMs[PAGE_SD_SLIDE]) {
    if (currentSdSlide < sdSlideCount) {
      currentSdSlide++;
      showSdSlidePage(tft, currentSdSlide, sdReady);
      pageStartedAt = millis();
    } else {
      enterNextEnabledPage();
    }
  }
}

void updateOnlineData() {
  if (WiFi.status() != WL_CONNECTED) {
    if (millis() - lastReconnectAttemptAt >= 30000) {
      WiFi.reconnect();
      lastReconnectAttemptAt = millis();
    }
    return;
  }

  if (millis() - lastPriceUpdateAt >= PRICE_REFRESH_MS) {
    fetchAllBitkubTickers();
    lastPriceUpdateAt = millis();
    if (currentPage == PAGE_COIN) showCoinPage(tft, coins[currentCoin], currentCoin);
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(TFT_BACKLIGHT_PIN, OUTPUT);
  digitalWrite(TFT_BACKLIGHT_PIN, HIGH);
  tft.init();
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);

  sdSpi.begin(SD_SCLK_PIN, SD_MISO_PIN, SD_MOSI_PIN, SD_CS_PIN);
  sdReady = SD.begin(SD_CS_PIN, sdSpi, 4000000);
  sdSlideCount = countSdSlides();
  loadSettings();
  Serial.printf("SD: %s, slides: %u\n", sdReady ? "ready" : "not found", sdSlideCount);

  wifiManager.setAPCallback(onWifiConfigMode);
  wifiManager.setConfigPortalTimeout(180);
  if (!wifiManager.autoConnect("CryptoClock-Setup")) {
    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_RED, TFT_BLACK);
    tft.drawCentreString("Wi-Fi setup timed out", 160, 105, 2);
    delay(3000);
    ESP.restart();
  }

  showWifiConnectedPage(tft, WiFi.localIP().toString());
  bootstrapMissingAssets();
  setupSettingsServer();
  configTime(7 * 3600, 0, "pool.ntp.org", "time.nist.gov");
  fetchAllBitkubTickers();
  lastPriceUpdateAt = millis();
  delay(500);
  enterPage(PAGE_PROFILE);
}

void loop() {
  settingsServer.handleClient();
  updateOnlineData();
  updatePageSequence();
  delay(10);
}
