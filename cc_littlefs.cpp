#include "LittleFS.h"
#include <ArduinoJson.h>
#include "cc_aes.h"
#include "GlobalParameter.h"

// CryptoClock File Manager
#include <ArduinoJson.h>
#include <LittleFS.h>


// ----------- config.json -----------
bool load_config() {
  Serial.println("Start Load Config");
  file = LittleFS.open("/config.json", "r");
  if (!file)
  {
    Serial.println("not found");
    return false;
  }
  delay(20);
  Serial.println("found!");
  delay(20);
  jsonData.clear();
  delay(20);
  file.seek(0, SeekSet);
  if (deserializeJson(jsonData, file) != DeserializationError::Ok)
  {
    Serial.println("not deserializeJson");
    Serial.println("----- RAW CONFIG FILE -----");
    while (file.available()) {
      Serial.write(file.read());
    }
    Serial.println("\n---------------------------");
    file.close();
    return false;
  }
  Serial.println(" deserializeJson");
  delay(200);
  firmwareVersion = jsonData["firmwareVersion"] | "4.1.4";
  deviceId = jsonData["deviceId"].isNull() ? "CCN000000" : jsonData["deviceId"].as<String>();
  plan = jsonData["plan"].as<String>();
  brightnessLevel = jsonData["brightnessLevel"] | 100;
  updateInterval = (jsonData["updateInterval"] | 300) * 1000;
  if (updateInterval < 10000 || updateInterval > 90000)
  {
    updateInterval = 10000 ;
  }
  timeout = jsonData["timeout"];
  pageIndex = jsonData["pageIndex"];
  coinIndex = jsonData["coinIndex"];
  currencyIndex = jsonData["currencyIndex"];
  pageMode = jsonData["pageMode"].as<String>();
  lineAlert = jsonData["lineAlert"] | false;
  soundAlert = jsonData["soundAlert"] | false;
  Serial.println("Start Load Config2");

  JsonObject durationObj = jsonData["pageDuration"];
  profileDuration = durationObj["profile"] | "15";
  coinDuration = durationObj["coin"] | "15";
  screensaverDuration = durationObj["screensaver"] | "15";
  cdcDuration = durationObj["cdc"] | "15";

  JsonObject toggleObj = jsonData["pageToggle"];
  profileToggle = toggleObj["profile"] | true;
  coinToggle = toggleObj["coin"] | true;
  screensaverToggle = toggleObj["screensaver"] | false;
  cdcToggle = toggleObj["cdc"] | false;

  // ตั้งค่า pageDurations
  pageDurations[0] = profileDuration.toInt() * 1000;
  pageDurations[1] = coinDuration.toInt() * 1000;
  pageDurations[2] = screensaverDuration.toInt() * 1000;
  pageDurations[3] = cdcDuration.toInt() * 1000;
  Serial.println("Start Load Config3");
  // เพิ่ม settings
  JsonObject settingsObj = jsonData["settings"];
  settings_alert = settingsObj["alert"];
  settings_game = settingsObj["game"];
  settings_horo = settingsObj["horo"];
  settings_plan = settingsObj["plan"].as<String>();
  maxCoin = settingsObj["coins"];
  region = jsonData["region"] | "default";
  timeZone = jsonData["timeZone"] | "Asia/Bangkok";
  currency1 = jsonData["currency1"] | "USDT";
  currency2 = jsonData["currency2"] | "THB";
  currency[0] = currency1;
  currency[1] = currency2;
  //  Serial.println("===== CONFIG LOADED ====="); delay(100);
  Serial.println("firmwareVersion: " + String(firmwareVersion)); delay(100);
  //  Serial.println("deviceId: " + deviceId); delay(100);
  //  Serial.println("plan: " + plan); delay(100);
  //  Serial.println("brightnessLevel: " + String(brightnessLevel)); delay(100);
  //  Serial.println("updateInterval: " + String(updateInterval)); delay(100);
  //  Serial.println("timeout: " + String(timeout)); delay(100);
  //  Serial.println("pageIndex: " + String(pageIndex)); delay(100);
  //  Serial.println("coinIndex: " + String(coinIndex)); delay(100);
  //  Serial.println("currencyIndex: " + String(currencyIndex)); delay(100);
  //  Serial.println("pageMode: " + String(pageMode)); delay(100);
  //  Serial.println("lotteryAlert: " + String(lotteryAlert)); delay(100);
  //  Serial.println("lineAlert: " + String(lineAlert)); delay(100);
  //  Serial.println("soundAlert: " + String(soundAlert)); delay(100);
  //  Serial.println("profileDuration: " + String(profileDuration)); delay(100);
  //  Serial.println("coinDuration: " + String(coinDuration)); delay(100);
  //  Serial.println("screensaverDuration: " + String(screensaverDuration)); delay(100);
  //  Serial.println("profileToggle: " + String(profileToggle)); delay(100);
  //  Serial.println("coinToggle: " + String(coinToggle)); delay(100);
  //  Serial.println("screensaverToggle: " + String(screensaverToggle)); delay(100);
  //  Serial.println("settings_alert: " + String(settings_alert)); delay(100);
  //  Serial.println("settings_game: " + String(settings_game)); delay(100);
  //  Serial.println("settings_horo: " + String(settings_horo)); delay(100);
  //  Serial.println("settings_plan: " + String(settings_plan)); delay(100);
  //  Serial.println("settings_coins: " + String(maxCoin)); delay(100);
  //  Serial.println("region: " + region);  delay(100);
  //  Serial.println("timeZone: " + timeZone);  delay(100);
  //  Serial.println("currency1: " + currency1); delay(100);
  //  Serial.println("currency2: " + currency2); delay(100);
  //  Serial.println("========================="); delay(100);
  file.close();
  return true;
}

bool save_config() {
  jsonData.clear();
  jsonData["firmwareVersion"] = firmwareVersion;
  jsonData["deviceId"] = deviceId;
  jsonData["plan"] = plan;
  jsonData["brightnessLevel"] = brightnessLevel;
  jsonData["updateInterval"] = updateInterval / 1000;
  jsonData["timeout"] = timeout;
  jsonData["pageIndex"] = pageIndex;
  jsonData["coinIndex"] = coinIndex;
  jsonData["currencyIndex"] = currencyIndex;
  jsonData["pageMode"] = pageMode;
  jsonData["region"] = region;
  jsonData["timeZone"] = timeZone;
  jsonData["currency1"] = currency1;
  jsonData["currency2"] = currency2;


  JsonObject duration = jsonData.createNestedObject("pageDuration");
  duration["profile"] = profileDuration;
  duration["coin"] = coinDuration;
  duration["screensaver"] = screensaverDuration;
  duration["cdc"] = cdcDuration;

  JsonObject toggle = jsonData.createNestedObject("pageToggle");
  toggle["profile"] = profileToggle;
  toggle["coin"] = coinToggle;
  toggle["screensaver"] = screensaverToggle;
  toggle["cdc"] = cdcToggle;

  jsonData["lineAlert"] = lineAlert;
  jsonData["soundAlert"] = soundAlert;



  // เพิ่ม settings
  JsonObject settingsObj = jsonData.createNestedObject("settings");
  settingsObj["alert"] = settings_alert;
  settingsObj["game"] = settings_game;
  settingsObj["horo"] = settings_horo;
  settingsObj["plan"] = settings_plan;
  settingsObj["coins"] = maxCoin;

  file = LittleFS.open("/config.json", "w");
  if (!file) return false;
  serializeJsonPretty(jsonData, file);
  file.close();
  return true;
}

bool save_default_config() {
  jsonData.clear();
  jsonData["firmwareVersion"] = "4.1.4";
  jsonData["deviceId"] = "CCN000000";
  jsonData["plan"] = "free";
  jsonData["brightnessLevel"] = 100;
  jsonData["updateInterval"] = 10;
  jsonData["timeout"] = 0;
  jsonData["pageIndex"] = 1;
  jsonData["coinIndex"] = 1;
  jsonData["currencyIndex"] = 0;
  jsonData["pageMode"] = "dynamic";
  jsonData["region"] = "default";
  jsonData["timeZone"] = "Asia/Bangkok";
  jsonData["currency1"] = "USDT";
  jsonData["currency2"] = "THB";


  JsonObject duration = jsonData.createNestedObject("pageDuration");
  duration["profile"] = "15";
  duration["coin"] = "15";
  duration["screensaver"] = "15";
  duration["cdc"] = "15";

  JsonObject toggle = jsonData.createNestedObject("pageToggle");
  toggle["profile"] = true;
  toggle["coin"] = true;
  toggle["screensaver"] = false;
  toggle["cdc"] = false;

  // เพิ่ม settings ค่า default
  JsonObject settingsObj = jsonData.createNestedObject("settings");
  settingsObj["alert"] = false;
  settingsObj["game"] = false;
  settingsObj["horo"] = false;
  settingsObj["plan"] = "free";
  settingsObj["coins"] = 4;

  jsonData["lotteryAlert"] = false;
  jsonData["lineAlert"] = false;
  jsonData["soundAlert"] = false;

  return save_config();
}


// ----------- wifi.json -----------
bool load_wifi() {
  Serial.println("เริ่มโหลดไฟล์ WiFi...");
  file = LittleFS.open("/wifi.json", "r");
  if (!file) {
    Serial.println("ไม่พบไฟล์ /wifi.json");
    return false;
  }
  Serial.println("ไฟล์ /wifi.json พบแล้ว");

  jsonData.clear();
  DeserializationError error = deserializeJson(jsonData, file);
  if (error != DeserializationError::Ok) {
    Serial.print("deserializeJson() ผิดพลาด: ");
    Serial.println(error.c_str());
    file.close();
    return false;
  }
  Serial.println("อ่าน JSON สำเร็จ");

  wifi_ssid = jsonData["ssid"].as<String>();
  wifi_password = jsonData["password"].as<String>();

  Serial.print("โหลด WiFi SSID: ");
  Serial.println(wifi_ssid);
  Serial.print("โหลด WiFi Password: ");
  Serial.println(wifi_password);

  file.close();
  return true;
}

bool save_wifi() {
  Serial.println("กำลังบันทึก WiFi...");

  jsonData.clear();
  jsonData["ssid"] = wifi_ssid;
  jsonData["password"] = wifi_password;

  file = LittleFS.open("/wifi.json", "w");
  if (!file) {
    Serial.println("ไม่สามารถเปิดไฟล์ /wifi.json เพื่อเขียนได้");
    return false;
  }

  serializeJsonPretty(jsonData, file);
  Serial.println("บันทึกข้อมูล WiFi ลงไฟล์สำเร็จ");

  file.close();
  return true;
}


bool save_default_wifi() {
  wifi_ssid = "";
  wifi_password = "";
  return save_wifi();
}

// ----------- profile.json -----------
bool load_profile() {
  file = LittleFS.open("/profile.json", "r");
  if (!file) return false;

  jsonData.clear();
  if (deserializeJson(jsonData, file) != DeserializationError::Ok) {
    file.close();
    return false;
  }
  file.close();

  // 🧍‍♂️ Profile
  profile_firstname      = jsonData["profile"]["firstname"].as<String>();
  profile_lastname       = jsonData["profile"]["lastname"].as<String>();
  profile_nickname       = jsonData["profile"]["nickname"].as<String>();
  profile_position       = jsonData["profile"]["position"].as<String>();
  profile_company        = jsonData["profile"]["company"].as<String>();
  profile_motto          = jsonData["profile"]["motto"].as<String>();

  // 🎨 Colors
  profile_bgcolor        = jsonData["colors"]["background"].as<String>();
  profile_namecolor      = jsonData["colors"]["name"].as<String>();
  profile_detailcolor    = jsonData["colors"]["detail"].as<String>();
  profile_mottocolor     = jsonData["colors"]["motto"].as<String>();
  profile_datetimecolor  = jsonData["colors"]["datetime"].as<String>();

  // 🧾 System
  profile_refer = jsonData["refer"].as<String>();
  if (jsonData.containsKey("referring")) {
    profile_referring.clear();
    deserializeJson(profile_referring, jsonData["referring"]);
  } else {
    profile_referring.clear();
  }

  // 📱 Contact
  contact_line     = jsonData["contact"]["line"].as<String>();
  contact_whatsapp = jsonData["contact"]["whatsapp"].as<String>();
  contact_tel      = jsonData["contact"]["tel"].as<String>();
  contact_email    = jsonData["contact"]["email"].as<String>();

  // 🌐 Social
  social_facebook  = jsonData["social"]["facebook"].as<String>();
  social_instagram = jsonData["social"]["instagram"].as<String>();
  social_youtube   = jsonData["social"]["youtube"].as<String>();
  social_tiktok    = jsonData["social"]["tiktok"].as<String>();
  Serial.print("profile_motto:");
  Serial.println(profile_motto);
  return true;
}

bool save_profile() {
  jsonData.clear();

  // 🧍‍♂️ ข้อมูลโปรไฟล์
  JsonObject profile = jsonData.createNestedObject("profile");
  profile["firstname"] = profile_firstname;
  profile["lastname"]  = profile_lastname;
  profile["nickname"]  = profile_nickname;
  profile["position"]  = profile_position;
  profile["company"]   = profile_company;
  profile["motto"]     = profile_motto;

  // 🎨 สี
  JsonObject colors = jsonData.createNestedObject("colors");
  colors["background"] = profile_bgcolor;
  colors["name"]       = profile_namecolor;
  colors["detail"]     = profile_detailcolor;
  colors["motto"]      = profile_mottocolor;
  colors["datetime"]   = profile_datetimecolor;

  // 🧾 ค่าระบบ
  jsonData["refer"]     = profile_refer;
  jsonData["referring"] = profile_referring;

  // 📱 Contact
  JsonObject contact = jsonData.createNestedObject("contact");
  contact["line"]     = contact_line;
  contact["whatsapp"] = contact_whatsapp;
  contact["tel"]      = contact_tel;
  contact["email"]    = contact_email;

  // 🌐 Social
  JsonObject social = jsonData.createNestedObject("social");
  social["facebook"]  = social_facebook;
  social["instagram"] = social_instagram;
  social["youtube"]   = social_youtube;
  social["tiktok"]    = social_tiktok;

  // 💾 เขียนลงไฟล์
  file = LittleFS.open("/profile.json", "w");
  if (!file) return false;

  serializeJsonPretty(jsonData, file);
  file.close();
  return true;
}


bool save_default_profile() {
  profile_firstname = "Satoshi";
  profile_lastname = "Nakamoto";
  profile_nickname = "Sat";
  profile_position = "Founder";
  profile_company = "Bitcoin";
  profile_motto = "We are all Satoshi.";
  profile_bgcolor = "#000000";
  profile_namecolor = "#FFAE00";
  profile_detailcolor = "#FFFFFF";
  profile_mottocolor = "#FFAE00";
  profile_datetimecolor = "#FFFFFF";
  profile_refer = "CCN000000";
  profile_referring.clear();

  jsonData.clear();

  JsonObject wallet = jsonData.createNestedObject("wallet");
  wallet["usdt"] = "0";
  wallet["btc"] = "0";
  wallet["cct"] = "0";
  wallet["stripe"] = "0";
  wallet["promptpay"] = "0000000000";

  JsonObject contact = jsonData.createNestedObject("contact");
  contact["line"] = "0";
  contact["whatsapp"] = "0";
  contact["tel"] = "0";
  contact["email"] = "mailto:mycryptoclock@gmail.com";

  JsonObject social = jsonData.createNestedObject("social");
  social["youtube"] = "https://youtube.com/@mycryptoclock";
  social["instagram"] = "https://instagram.com/mycryptoclock";
  social["facebook"] = "https://facebook.com/mycryptoclock";
  social["tiktok"] = "https://tiktok.com/@mycryptoclock";

  return save_profile();
}


// ----------- usage.json -----------
bool load_usage() {
  file = LittleFS.open("/usage.json", "r");
  if (!file) return false;
  jsonData.clear();
  if (deserializeJson(jsonData, file) != DeserializationError::Ok) return false;
  usage_total = jsonData["usage_total"];
  return true;
}

bool save_usage() {
  jsonData.clear();
  jsonData["usage_total"] = usage_total;
  file = LittleFS.open("/usage.json", "w");
  if (!file) return false;
  serializeJsonPretty(jsonData, file);
  return true;
}

bool save_default_usage() {
  usage_total = 0;
  return save_usage();
}

// ----------- alert.json -----------
bool load_alert() {
  Serial.println("เริ่มโหลด alert");
  file = LittleFS.open("/alert.json", "r");
  if (!file) {
    Serial.println("ไม่พบไฟล์ alert.json ใช้ค่ามาตรฐานแทน");
    return false;
  }

  jsonData.clear();
  if (deserializeJson(jsonData, file) != DeserializationError::Ok) {
    Serial.println("ไม่สามารถอ่าน JSON ได้ ใช้ค่ามาตรฐานแทน2");
    return false;
  }

  if (jsonData.containsKey("currency")) {
    
    currencyAlert = jsonData["currency"].as<String>();
    Serial.print("currencyAlert : ");
     Serial.println(currencyAlert);
  } else {
    currencyAlert = "USDT"; // กำหนดค่ามาตรฐาน
    Serial.println("ไม่พบ currency ใน JSON ใช้ค่า default: USD");
  }

  if (jsonData.containsKey("alertEnable")) {
    alertEnable = jsonData["alertEnable"].as<bool>();
    Serial.print("alertEnable : ");
     Serial.println(alertEnable);
  } else {
    alertEnable = false; // กำหนดค่ามาตรฐาน
    Serial.println("ไม่พบ alertEnable ใน JSON ใช้ค่า default: false");
  }

   if (jsonData.containsKey("lotteryAlert")) {
    lotteryAlert = jsonData["lotteryAlert"].as<bool>();
     Serial.print("lotteryAlert : ");
     Serial.println(lotteryAlert);
  } else {
    lotteryAlert = false; // กำหนดค่ามาตรฐาน
    Serial.println("ไม่พบ lotteryAlert ใน JSON ใช้ค่า default: false");
  }

  if (!jsonData.containsKey("alerts")) {
    Serial.println("ไม่พบ alerts ใน JSON ใช้ค่ามาตรฐานทั้งหมด");
    return false;
  }

  JsonArray alerts = jsonData["alerts"];
  for (int i = 0; i < 4; i++) {
    JsonObject alert = alerts[i];
    JsonArray highs = alert["high"];
    JsonArray lows = alert["low"];
    // --- โหลด High ---
    int hiCount = highs.size();
    for (int j = 0; j < hiCount && j < 8; j++) {
      if (highs[j].containsKey("value")) {
        coinHigh[i][j] = highs[j]["value"].as<float>();
      } else {
        coinHigh[i][j] = 10000000.0;
        Serial.printf("high[%d][%d] ไม่มีค่า กำหนด default: 10000000.0\n", i, j);
      }

      coinHighStatus[i][j] = highs[j].containsKey("status") ? highs[j]["status"].as<bool>() : false;
    }
    alertCountHigh[i] = hiCount;

    // --- โหลด Low ---
    int loCount = lows.size();
    for (int j = 0; j < loCount && j < 8; j++) {
      if (lows[j].containsKey("value")) {
        coinLow[i][j] = lows[j]["value"].as<float>();
      } else {
        coinLow[i][j] = 0.0;
        Serial.printf("low[%d][%d] ไม่มีค่า กำหนด default: 0.0\n", i, j);
      }

      coinLowStatus[i][j] = lows[j].containsKey("status") ? lows[j]["status"].as<bool>() : false;
    }
    alertCountLow[i] = loCount;

  }

  return true;
}



bool save_alert() {
  File file = LittleFS.open("/alert.json", "w");
  if (!file) return false;

  DynamicJsonDocument doc(8192);
  doc["currency"] = currencyAlert;
  doc["lotteryAlert"] = lotteryAlert;
  doc["alertEnable"] = alertEnable;

  JsonArray alerts = doc.createNestedArray("alerts");

  for (int i = 0; i < 4; i++) {
    JsonObject obj = alerts.createNestedObject();
    obj["coinIndex"] = i;

    JsonArray high = obj.createNestedArray("high");
    JsonArray low  = obj.createNestedArray("low");

    // 🔺 เพิ่มเฉพาะเท่าที่ alertCountHigh[i]
    for (int j = 0; j < alertCountHigh[i]; j++) {
      JsonObject h = high.createNestedObject();
      h["value"] = coinHigh[i][j];
      h["status"] = coinHighStatus[i][j];
    }

    // 🔻 เพิ่มเฉพาะเท่าที่ alertCountLow[i]
    for (int j = 0; j < alertCountLow[i]; j++) {
      JsonObject l = low.createNestedObject();
      l["value"] = coinLow[i][j];
      l["status"] = coinLowStatus[i][j];
    }
  }

  bool ok = serializeJson(doc, file) > 0;
  file.close();
  return ok;
}




bool save_default_alert() {
  Serial.println("save_default_alert");
  currencyAlert = "USDT";
  alertEnable = false;
  for (int i = 0; i < 4; i++) {
    for (int j = 0; j < 8; j++) {
      coinHigh[i][j] = 10000000.0;
      coinHighStatus[i][j] = false;
      coinLow[i][j] = 0.0;
      coinLowStatus[i][j] = false;
    }
  }

  return save_alert();
}


bool load_coin() {
  file = LittleFS.open("/coin.json", "r");
  if (!file) {
    Serial.println("Failed to open /coin.json for reading");
    return false;
  }

  file.seek(0, SeekSet);  // ย้าย pointer ไปตำแหน่งเริ่มต้น

  jsonData.clear();
  DeserializationError error = deserializeJson(jsonData, file);
  file.close();

  if (error) {
    Serial.println("Failed to parse /coin.json");
    return false;
  }

  // อ่านข้อมูลเหรียญทั้ง 4 จาก JSON
  JsonArray coinsArray = jsonData["coins"];
  for (size_t i = 0; i < coinsArray.size() && i < 4; i++) {
    JsonObject coinData = coinsArray[i];
    coin[i] = coinData["name"].as<String>();
    type[i] = coinData["type"].as<String>();
    color[i] = coinData["color"].as<String>();
    market[i] = coinData["market"].as<String>();

    Serial.print("[COIN] ");
    Serial.print(coin[i]);
    Serial.print(" - Type: ");
    Serial.print(type[i]);
    Serial.print(" - Color: ");
    Serial.println(color[i]);
    Serial.print(" - Market: ");
    Serial.println(market[i]);
  }

  coinType = type[coinIndex].toInt();
  Serial.println("Coins loaded successfully from /coin.json");
  return true;
}

bool save_coin() {
  jsonData.clear();
  JsonArray coinsArray = jsonData.createNestedArray("coins");

  for (int i = 0; i < 4; i++) {
    JsonObject obj = coinsArray.createNestedObject();
    obj["name"] = coin[i];
    obj["type"] = type[i];
    obj["color"] = color[i];
    obj["market"] = market[i];
  }

  file = LittleFS.open("/coin.json", "w");
  if (!file) return false;
  serializeJsonPretty(jsonData, file);
  file.close();
  return true;
}

bool save_default_coin() {
  jsonData.clear();
  JsonArray arr = jsonData.createNestedArray("coins");

  JsonObject obj1 = arr.createNestedObject();
  obj1["name"] = "BTC";
  obj1["type"] = 3;
  obj1["color"] = "#FFD700";
  obj1["market"] = "NASDAQ";

  JsonObject obj2 = arr.createNestedObject();
  obj2["name"] = "KUB";
  obj2["type"] = 2;
  obj2["color"] = "#00FF00";
  obj2["market"] = "NASDAQ";

  JsonObject obj3 = arr.createNestedObject();
  obj3["name"] = "BNB";
  obj3["type"] = 3;
  obj3["color"] = "#FFD700";
  obj3["market"] = "NASDAQ";

  JsonObject obj4 = arr.createNestedObject();
  obj4["name"] = "DOGE";
  obj4["type"] = 3;
  obj4["color"] = "#FFD700";
  obj4["market"] = "NASDAQ";

  return save_coin();
}



// ----------- ads.json -----------
bool load_ads() {
  file = LittleFS.open("/ads.json", "r");
  if (!file) return false;
  jsonData.clear();
  if (deserializeJson(jsonData, file) != DeserializationError::Ok) return false;
  ads_enabled = jsonData["adsEnabled"];
  current_ads = jsonData["currentAds"].as<String>();
  return true;
}

bool save_ads() {
  jsonData.clear();
  jsonData["adsEnabled"] = ads_enabled;
  jsonData["currentAds"] = current_ads;
  file = LittleFS.open("/ads.json", "w");
  if (!file) return false;
  serializeJsonPretty(jsonData, file);
  return true;
}

bool save_default_ads() {
  ads_enabled = true;
  current_ads = "0";
  return save_ads();
}

// ----------- รวมทุกไฟล์ -----------
void load_all() {
  load_config();
  load_wifi();
  load_profile();
  load_usage();
  load_alert();
  load_coin();
  load_ads();
}

void save_all() {
  save_config();
  save_wifi();
  save_profile();
  save_usage();
  save_alert();
  save_coin();
  save_ads();
}

void save_all_defaults() {
  save_default_config();
  save_default_wifi();
  save_default_profile();
  save_default_usage();
  save_default_alert();
  save_default_coin();
  save_default_ads();
}

void check_and_create_all() {
  if (!LittleFS.exists("/config.json")) save_default_config();
  if (!LittleFS.exists("/wifi.json")) save_default_wifi();
  if (!LittleFS.exists("/profile.json")) save_default_profile();
  if (!LittleFS.exists("/usage.json")) save_default_usage();
  if (!LittleFS.exists("/alert.json")) save_default_alert();
  if (!LittleFS.exists("/coin.json")) save_default_coin();
  if (!LittleFS.exists("/ads.json")) save_default_ads();
}
