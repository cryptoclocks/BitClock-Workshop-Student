#include <Arduino.h>
#include "GlobalModule.h"
#include "GlobalParameter.h"
#include "GlobalFunction.h"
#include <XPT2046_Bitbang.h>
#include "cc_littlefs.h"
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <FS.h>
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

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <string>
#include <iostream>
#include "qrcode_gen.h"


//rates

unsigned long lastTouchTime = 0;
const unsigned long debounceTime = 500; // 500 milliseconds for debounce
static int touchCounter = 0;

bool wasTouched = false;
TouchPoint lastTouch = {0, 0, 0}; // Last valid touch point
TouchPoint maxZTouch;


WiFiClientSecure client; // สร้างครั้งเดียว
HTTPClient http3;
#define SECS_PER_DAY 86400UL

// BEEP TEST:
// Keep skip at 0 for normal behavior. Change to 1 temporarily to confirm
// whether starting the price-fetch task is causing the page-2 chirp.
#define PAGE2_BEEP_TEST_CLAMP_TASK_BOUNDARY 1
#define PAGE2_BEEP_TEST_SKIP_PRICE_TASK_START 0

//page

DeserializationError errFetch ;



#define PASTEL_ARROW    tft.color565(150, 150, 200) // Soft blue-gray arrow

// Function to parse CSV data
void parseCSV(String data) {
  int lineEnd = data.indexOf('\n');
  data = data.substring(lineEnd + 1); // Skip the header line

  for (int i = 0; i < numChannels && data.length() > 0; i++) {
    lineEnd = data.indexOf('\n');
    String line = (lineEnd > 0) ? data.substring(0, lineEnd) : data;
    data = (lineEnd > 0) ? data.substring(lineEnd + 1) : "";

    // Split the line into columns based on commas
    int firstComma = line.indexOf(',');
    int secondComma = line.indexOf(',', firstComma + 1);

    float rate = line.substring(secondComma + 1).toFloat();
    rates[i] = rate;
  }
}


// Function to fetch and parse CSV data
void fetchAndParseCSV(String url) {
  http3;
  http3.begin(url);
  int httpResponseCode = http3.GET();
  Serial.println("HTTP Response Code: " + String(httpResponseCode));

  if (httpResponseCode > 0) {
    String payload = http3.getString();
    parseCSV(payload);
  } else {
    Serial.println("Error on HTTP request");
  }
  http3.end();
}



//task
// กำหนด Prototype ของฟังก์ชันที่จะใช้เป็น Task
TaskHandle_t priceTaskHandle = NULL;

void fetchAndDisplayPriceTask(void *parameter);

bool isNumeric(String str) {
  for (byte i = 0; i < str.length(); i++) {
    if (!isDigit(str.charAt(i))) return false;
  }
  return true;
}

// Google Apps Script registration and profile upload removed.

// Function to remove commas and decimals, then convert to integer
int convertToInt(String str) {
  str.replace(",", ""); // Remove commas
  int dotIndex = str.indexOf('.'); // Find the decimal point
  if (dotIndex != -1) {
    str = str.substring(0, dotIndex); // Remove the decimal part
  }
  return str.toInt(); // Convert to integer
}

void startPriceTask() {
  Serial.println("Start Task");
#if PAGE2_BEEP_TEST_CLAMP_TASK_BOUNDARY
  silenceBuzzer();
#endif
#if PAGE2_BEEP_TEST_SKIP_PRICE_TASK_START
  if (pageIndex == 2) {
    Serial.println("[BEEP TEST] Skip startPriceTask on page 2");
    return;
  }
#endif
  if (priceTaskHandle == NULL) {
    isTaskOn = true;
    xTaskCreate(fetchAndDisplayPriceTask, "Fetch Price", 10000, NULL, 1, &priceTaskHandle);
  }
#if PAGE2_BEEP_TEST_CLAMP_TASK_BOUNDARY
  silenceBuzzer();
#endif
}

void stopPriceTask() {
  Serial.println("Stop Task");
#if PAGE2_BEEP_TEST_CLAMP_TASK_BOUNDARY
  silenceBuzzer();
#endif
  if (priceTaskHandle != NULL) {
    vTaskDelete(priceTaskHandle);
    memset(&priceTaskHandle, 0, sizeof(priceTaskHandle));
    priceTaskHandle = NULL;
    Serial.println("Task stopped successfully and handle is set to NULL");
  } else {
    Serial.println("Task handle is already NULL, no task to stop");
  }
#if PAGE2_BEEP_TEST_CLAMP_TASK_BOUNDARY
  silenceBuzzer();
#endif
}

// ช่วยแปลง String -> int แบบปลอดภัย
int toIntSafe(const String& s) {
  String t = s;
  t.replace(",", "");
  t.trim();
  if (t.length() == 0) return -1;
  bool allDigitOrDot = true;
  for (size_t i = 0; i < t.length(); i++) {
    char c = t[i];
    if (!isDigit(c) && c != '.' && c != '-') {
      allDigitOrDot = false;
      break;
    }
  }
  if (!allDigitOrDot) return -1;
  // ปัดเศษทศนิยม (ข้อมูลบางอันเป็น "1834.20")
  return (int)round(t.toFloat());
}

// หา object ตามชื่อ แล้วคืนค่า field ที่ต้องการ
String getFieldByName(JsonArray arr, const char* targetName, const char* field /* "bid"|"ask"|"diff" */) {
  for (JsonObject obj : arr) {
    const char* n = obj["name"] | "";
    if (strcmp(n, targetName) == 0) {
      // บางค่ามาเป็นตัวเลข/สตริงปนกัน ใช้ as<String>() ให้หมด
      return obj[field].as<String>();
    }
  }
  return String();
}

void deserializeAndPrintGoldPrices(const String& json) {
  DynamicJsonDocument doc(16384);

  doc.clear();
  DeserializationError errFetch = deserializeJson(doc, json);
  if (errFetch) {
    Serial.print("Failed to parse JSON: ");
    Serial.println(errFetch.c_str());
    return;
  }

  if (!doc.is<JsonArray>()) {
    Serial.println("Unexpected JSON format (expected array).");
    return;
  }

  JsonArray arr = doc.as<JsonArray>();

  // ทองแท่ง สมาคมฯ (ซื้อเข้า/ขายออก)
  String bar_buy_str  = getFieldByName(arr, "สมาคมฯ", "bid");
  String bar_sell_str = getFieldByName(arr, "สมาคมฯ", "ask");

  bar_buy  = toIntSafe(bar_buy_str);
  bar_sell = toIntSafe(bar_sell_str);

  // ทองรูปพรรณ — ฟีดนี้ไม่มีตรง → เว้นไว้ก่อน
  jewelry_buy  = -1;
  jewelry_sell = -1;

  Serial.print("Gold Bar - Buy: ");
  Serial.println(bar_buy);
  Serial.print("Gold Bar - Sell: ");
  Serial.println(bar_sell);
  Serial.print("Jewelry Gold - Buy: ");
  Serial.println(jewelry_buy);
  Serial.print("Jewelry Gold - Sell: ");
  Serial.println(jewelry_sell);
}

void deserializeAndPrintGoldPrices_Streamed() {
  // ใช้ค่าจาก buffer ที่รันระหว่าง fetch

  Serial.print("Gold Bar - Buy: ");   Serial.println(bar_buy);
  Serial.print("Gold Bar - Sell: ");  Serial.println(bar_sell);
  Serial.print("Jewelry - Buy: ");    Serial.println(jewelry_buy);
  Serial.print("Jewelry - Sell: ");   Serial.println(jewelry_sell);
}

bool extractNumberAfterId(const String& buf, const char* id, int &outVal) {
  int k = buf.indexOf(id);
  if (k < 0) return false;
  // ตัดเฉพาะส่วนหลัง id แล้วหาเลขก้อนแรก
  int start = k;
  // หา '>' ตัวแรกหลัง id
  start = buf.indexOf('>', start);
  if (start < 0) return false;
  // ข้ามไปจนถึงตัวเลขตัวแรก
  int i = start + 1;
  while (i < (int)buf.length() && !(isDigit(buf[i]) || buf[i] == '๐' || buf[i] == '๑' || buf[i] == '๒' || buf[i] == '๓' || buf[i] == '๔' || buf[i] == '๕' || buf[i] == '๖' || buf[i] == '๗' || buf[i] == '๘' || buf[i] == '๙')) i++;
  if (i >= (int)buf.length()) return false;
  int j = i;
  while (j < (int)buf.length() && (isDigit(buf[j]) || buf[j] == ',' || buf[j] == '.' ||
                                   buf[j] == '๐' || buf[j] == '๑' || buf[j] == '๒' || buf[j] == '๓' || buf[j] == '๔' || buf[j] == '๕' || buf[j] == '๖' || buf[j] == '๗' || buf[j] == '๘' || buf[j] == '๙')) j++;
  String num = buf.substring(i, j);
  outVal = toIntSafe(num);
  return true;
}

// Optional legacy helper: intentionally blank in the public student package.
const char* ssid = "";
const char* password = "";

// ====== Fetch (อ่านทีละบรรทัด + break เมื่อครบ 4 ค่าแบบ valid เท่านั้น) ======
void fetchGoldPrices() {

  Serial.println("Start Fetch Gold");
  WiFiClientSecure tls;
  tls.setInsecure();                           // ถ้าไม่มี cert
  http3.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
  http3.setTimeout(15000);
  http3.setUserAgent("ESP32");                 // กันบางเว็บ block
  http3.addHeader("Accept-Encoding", "identity");
  http3.begin("https://www.goldtraders.or.th/default.aspx");
  int httpResponseCode = http3.GET();
  Serial.println(httpResponseCode);

  if (httpResponseCode == HTTP_CODE_OK) {
    WiFiClient* stream = http3.getStreamPtr();

    String buf;            // rolling buffer เล็ก ๆ
    buf.reserve(3000);

    // ไม่ reset ค่าเดิม (เผื่อใช้ค่าล่าสุดที่ดีอยู่แล้ว)
    bool gotBLBuy = false, gotBLSell = false, gotOMBuy = false, gotOMSell = false;

    while (stream->connected()) {
      if (!stream->available()) {
        if (http3.connected()) {
          delay(5);
          continue;
        } else {
          break;
        }
      }

      String line = stream->readStringUntil('\n');
      if (line.length()) {
        line.replace("\r", "");
        buf += line;

        // จำกัดขนาด buffer ประมาณ 3KB
        if (buf.length() > 3000) buf.remove(0, buf.length() - 3000);
      }

      // ----- ดึงค่าเฉพาะเมื่อ v != -1 -----
      if (!gotBLSell) {
        int v;
        if (extractNumberAfterId(buf, "DetailPlace_uc_goldprices1_lblBLSell", v)) {
          if (v != -1) {
            bar_sell = v;
            gotBLSell = true;
          }
        }
      }

      if (!gotBLBuy) {
        int v;
        if (extractNumberAfterId(buf, "DetailPlace_uc_goldprices1_lblBLBuy", v)) {
          if (v != -1) {
            bar_buy = v;
            gotBLBuy = true;
          }
        }
      }

      if (!gotOMSell) {
        int v;
        if (extractNumberAfterId(buf, "DetailPlace_uc_goldprices1_lblOMSell", v)) {
          if (v != -1) {
            jewelry_sell = v;
            gotOMSell = true;
          }
        }
      }

      if (!gotOMBuy) {
        int v;
        if (extractNumberAfterId(buf, "DetailPlace_uc_goldprices1_lblOMBuy", v)) {
          if (v != -1) {
            jewelry_buy = v;
            gotOMBuy = true;
          }
        }
      }

      // ได้ครบ 4 ค่าแบบ "valid" แล้วค่อยหยุด
      if (gotBLBuy && gotBLSell && gotOMBuy && gotOMSell) break;
    }

    deserializeAndPrintGoldPrices_Streamed();

  } else {
    Serial.print("Error on HTTP request: ");
    Serial.println(httpResponseCode);
    String response = http3.getString();
    Serial.println(response);
    http3.end();
  }

  http3.end();
}

bool firsterr = false ;

float fetchExchangeRate(String from, String to) {
  if (to == "USDT") to = "USD";
  if (from == "USDT") from = "USD";
  if (to == "USDC") to = "USD";
  if (from == "USDC") from = "USD";

  if (from == to) return 1.0; // ไม่ต้องแปลง

  // เพิ่มเงื่อนไขสำหรับสกุลเงินที่อ่อนมากๆ
  if (to == "LBP") return 89600.0;   // Lebanon Pound: 1 USD = 89,600 LBP
  if (to == "IRR") return 42100.0;   // Iran Rial: 1 USD = 42,100 IRR
  if (to == "UZS") return 10911.0;   // Uzbekistan Som: 1 USD = 10,911 UZS
  if (to == "SYP") return 2500.0;    // Syrian Pound: 1 USD = 2,500 SYP
  if (to == "GNF") return 8655.0;    // Guinea Franc: 1 USD = 8,655 GNF
  if (to == "PYG") return 7000.0;    // Paraguay Guarani: 1 USD = 7,000 PYG
  if (to == "MGA") return 4000.0;    // Madagascar Ariary: 1 USD = 4,000 MGA
  if (to == "KHR") return 4000.0;    // Cambodia Riel: 1 USD = 4,000 KHR
  if (to == "BIF") return 2000.0;    // Burundi Franc: 1 USD = 2,000 BIF
  if (to == "ZWL") return 2047.0;    // Zimbabwe Dollar: 1 USD = 2,047 ZWL

  String url = "https://api.exchangerate-api.com/v4/latest/" + from;
  http3.begin(url);
  int httpCode = http3.GET();

  if (httpCode > 0) {
    String payload = http3.getString();

    if (payload.length() == 0) {
      Serial.println("Empty payload");
      http3.end();
      return 1.0;
    }

    DynamicJsonDocument doc(16384); // หรือมากกว่า
    DeserializationError err = deserializeJson(doc, payload);
    if (!err) {
      if (doc["rates"].containsKey(to)) {
        float rate = doc["rates"][to].as<float>();
        http3.end();
        return rate;
      } else {
        Serial.println("Currency not found in rates");
      }
    } else {
      Serial.println("JSON parse failed: " + String(err.c_str()));
    }
  } else {
    Serial.println("HTTP request failed: " + String(httpCode));
  }

  http3.end();
  return 1.0; // fallback กรณีมีปัญหาทุกกรณี
}


void updateDisplayValues() {
  for (int i = 0; i < 4; i++) {
    displayPrice[i]         = price[i]         * exRate[i][currencyIndex];
    displayHighPrice[i]     = highPrice[i]     * exRate[i][currencyIndex];
    displayLowPrice[i]      = lowPrice[i]      * exRate[i][currencyIndex];
    displayChange[i]        = priceChange[i]   * exRate[i][currencyIndex];
    displayChangePercent[i] = priceChangePercent[i]; // ไม่ต้องแปลง

    Serial.print("Coin Index "); Serial.println(i);
    Serial.print("  Price: "); Serial.println(price[i]);
    Serial.print("  ExRate: "); Serial.println(exRate[i][currencyIndex]);
    Serial.print("  Display Price: "); Serial.println(displayPrice[i]);
    Serial.println("----------------------");
  }
}



void updateExchangeRate() {
  for (int i = 0; i < 4; i++) {           // วิ่งตามเหรียญ
    if (coin[i] == "XAU") {
      exRate[i][0] = 1.0;      // USD→USD
      exRate[i][1] = fetchExchangeRate("USD", currency2); // USD→THB
    }
    else
    {

      int tempType = type[i].toInt();
      String from = (tempType == 2 || (tempType == 3 && currencyIndex == 1) || (tempType == 4 && market[i] == "SET")) ? "THB" : "USD";

      for (int j = 0; j < 2; j++) {         // วิ่งตามสกุลเงิน 0=currency1, 1=currency2
        String to = currency[j];

        Serial.printf("ขออัตราแลกเปลี่ยน %s → %s สำหรับ coin[%d] = %s\n", from.c_str(), to.c_str(), i, coin[i].c_str());

        exRate[i][j] = fetchExchangeRate(from, to);

        Serial.printf("exRate[%d][%d] = %.4f\n", i, j, exRate[i][j]);
        //delay(50); // ⭐️ ลดเหลือ delay 500ms ก็พอ ไม่งั้น fetch ช้าเกิน
      }
    }
  }
}

void handleJsonError(DeserializationError& error) {
  Serial.print(F("deserializeJson() failed: "));
  Serial.println(error.f_str());
  waitforprice = false;
}

void handleError(int httpResponseCode) {
  Serial.print("Error on sending POST: ");
  Serial.println(httpResponseCode);
  if (httpResponseCode == -1) {
    ESP.restart();
  }
}

//api function
// ฟังก์ชันแทน {symbol} ด้วยเหรียญที่ต้องการ (แทนที่ placeholder ใน config เมื่อใช้งาน)
String replaceSymbol(String url, String symbol) {
  url.replace("{symbol}", symbol);
  return url;
}


void updatePricesFromXAU(DynamicJsonDocument& doc, int i) {
  if (i < 0 || i > 3) return;          // ป้องกัน index เกิน

  float p = doc["price"].as<float>();  // ดึงราคาทอง

  price[i]              = p;   // เก็บเป็น base-currency
  highPrice[i]          = 0;
  lowPrice[i]           = 0;
  priceChange[i]        = 0;
  priceChangePercent[i] = 0;

  updateDisplayValues();       // คูณ exRate และอัปเดตจอ

  Serial.printf("XAU %.4f USD → %.4f %s\n",
                p, displayPrice[i], currency[currencyIndex].c_str());
}


void setupAPIForCoin(int i) {

  int coinType = type[i].toInt();   // แปลง String → int
  String mkt   = market[i];         // ตลาด (เช่น "SET", "NASDAQ")
  Serial.println("xxxxxxxxxxxxxxx");
  Serial.print("coinType");
  Serial.println(coinType);
  Serial.print("currencyIndex");
  Serial.println(currencyIndex);
  Serial.println("xxxxxxxxxxxxxxx");
  if (coinType == 0 && coin[i] == "JBC") {
    api = "https://api.jibswap.com/api/v1/jbc_price";
  } else if (coinType == 0 && coin[i] == "XAU") {
    api = "https://api.gold-api.com/price/XAU";
  } else if (coin[i] == "BGB") {
    api = "https://api.bitget.com/api/v2/spot/market/tickers?symbol=BGBUSDT";
  } else if (coinType == 1 ) {
    //api = "https://api.binance.com/api/v3/ticker/24hr?symbol=" + coin[i] + "USDT";
    api = replaceSymbol(api_url[0], coin[i]);
    Serial.print("API (currency1) for ");
    Serial.print(coin[i]);
    Serial.print(": ");
    Serial.println(api);
  } else if (coinType == 2) { //delete bitkub
    api = replaceSymbol(api_url[1], coin[i]);
    Serial.print("API (currency2) for ");
    Serial.print(coin[i]);
    Serial.print(": ");
    Serial.println(api);

  } else if (coinType == 3) {

    //api = "https://api.binance.com/api/v3/ticker/24hr?symbol=" + coin[i] + "USDT";
    if (currencyIndex == 0)
    {
      api = replaceSymbol(api_url[0], coin[i]);
    }
    else  if (currencyIndex == 1)
    {
      api = replaceSymbol(api_url[1], coin[i]);
    }

    Serial.print("API (currency1) for ");
    Serial.print(coin[i]);
    Serial.print(": ");
    Serial.println(api);

  } else if (coinType == 4) { //thai

    // หุ้นไทย (SET) ต่อ .BK, ที่เหลือไม่ต่อ
    api = "https://query2.finance.yahoo.com/v8/finance/chart/" +
          coin[i] + (mkt == "SET" ? ".BK" : "") +
          "?interval=1d";

  } else if (coinType == 5) {
    api = "https://query2.finance.yahoo.com/v8/finance/chart/" + coin[i] + "?interval=1d" ;
  } else if (coinType == 6) { //currency
    String currencyX = (currencyIndex == 0) ? "USD" : currency[currencyIndex];
    api = "https://api.frankfurter.app/latest?from=" + coin[i] + "&to=" + currencyX;
  } else if (coinType == 7) { //indice
    api = "https://query2.finance.yahoo.com/v8/finance/chart/" +
          coin[i] + "?interval=1d";
  }
  Serial.print("API:") ;
  Serial.println(api) ;
}

void updatePricesFromJBC(DynamicJsonDocument& doc, int i) {
  if (i >= 0 && i <= 3) {
    float p = doc["jbc_price"].as<float>();

    // JBC ส่งมาเป็น USDT เสมอ → เก็บแบบ base
    price[i] = p;
    lowPrice[i] = 0;
    highPrice[i] = 0;
    priceChange[i] = 0;
    priceChangePercent[i] = 0;

    updateDisplayValues(); // แปลงค่าแสดงผล

    Serial.printf("JBC ราคา %.4f USDT → %.4f %s\n", p, displayPrice[i], currency[currencyIndex].c_str());
  }
}




// ฟังก์ชันช่วย: ดึงค่าจาก JSON แบบ nested โดยใช้ path ที่เป็น string ที่มีเครื่องหมายจุด ('.')
float getValueFromJSON(DynamicJsonDocument &doc, String path) {
  JsonVariant value = doc;

  int start = 0;
  while (true) {
    int dot = path.indexOf('.', start);
    String key = (dot == -1)
                   ? path.substring(start)
                   : path.substring(start, dot);

    key.trim();

    // ===== รองรับ array index =====
    if (key.length() && isDigit(key[0])) {
      int idx = key.toInt();
      if (!value.is<JsonArray>() || idx >= value.size()) {
        Serial.print("Array index not found: ");
        Serial.println(key);
        return 0;
      }
      value = value[idx];
    }
    // ===== รองรับ object key =====
    else {
      if (!value.is<JsonObject>() || !value.containsKey(key)) {
        Serial.print("Key not found: ");
        Serial.println(key);
        return 0;
      }
      value = value[key];
    }

    if (dot == -1) break;
    start = dot + 1;
  }

  return value.as<float>();
}


// ฟังก์ชัน updatePrices สำหรับดึงข้อมูลราคาเหรียญจาก API dynamic
void updatePrices(DynamicJsonDocument &doc, int i) {
  String symbol = coin[i];
  int idx = 0;
  if (api.indexOf(configKeys[1]) != -1) {
    idx = 1;
  }

  String finalPricePath    = replaceSymbol(price_path[idx], symbol);
  String finalHighPath     = replaceSymbol(high_path[idx], symbol);
  String finalLowPath      = replaceSymbol(low_path[idx], symbol);
  String finalChangePath   = replaceSymbol(change_path[idx], symbol);
  String finalPercentPath  = replaceSymbol(percent_path[idx], symbol);

  float p      = getValueFromJSON(doc, finalPricePath);
  float high   = getValueFromJSON(doc, finalHighPath);
  float low    = getValueFromJSON(doc, finalLowPath);
  float change = getValueFromJSON(doc, finalChangePath);
  float percent = getValueFromJSON(doc, finalPercentPath);

  Serial.println("----- Stock Data -----");
  Serial.print("Price: "); Serial.println(p);
  Serial.print("High: "); Serial.println(high);
  Serial.print("Low: "); Serial.println(low);
  Serial.print("Change: "); Serial.println(change);
  Serial.print("Percent Change: "); Serial.println(percent);
  Serial.println("----------------------");

  // บันทึกข้อมูล base currency เท่านั้น (ยังไม่คูณ exRate)
  price[i] = p;
  highPrice[i] = high;
  lowPrice[i] = low;
  priceChange[i] = change;
  priceChangePercent[i] = percent;

  Serial.print("Coin: ");
  Serial.print(coin[i]);
  Serial.print(" | Price (base): ");
  Serial.println(p);

  // คูณ exRate ทีหลัง
  updateDisplayValues();
}



void updatePricesFromCrypto(DynamicJsonDocument & doc, int i) {
  updatePrices(doc, i);
}

void updatePricesFromBitget(DynamicJsonDocument& doc, int i) {
  JsonArray data = doc["data"];
  if (data.size() == 0) return;

  JsonObject obj = data[0];

  float p       = obj["lastPr"].as<float>();
  float high    = obj["high24h"].as<float>();
  float low     = obj["low24h"].as<float>();
  float open    = obj["open"].as<float>();
  float change  = p - open;
  float percent = (change / open) * 100.0;

  price[i] = p;
  highPrice[i] = high;
  lowPrice[i] = low;
  priceChange[i] = change;
  priceChangePercent[i] = percent;

  updateDisplayValues();

  Serial.printf("BGB %.4f → %.4f %s\n", p, displayPrice[i], currency[currencyIndex].c_str());
}


void parseCryptoPriceResponse(DynamicJsonDocument & doc, int i) {
  if (coinType == 0 && coin[i] == "JBC") {
    updatePricesFromJBC(doc, i);
  } else if (coinType == 0 && coin[i] == "XAU") {
    // ⭐️ เพิ่มตรงนี้
    updatePricesFromXAU(doc, i);
  } else if (coin[i] == "BGB") {
    updatePricesFromBitget(doc, i);
  } else if (coinType == 1 || coinType == 2 || coinType == 3) {
    Serial.println("updatePricesFromCrypto");
    updatePricesFromCrypto(doc, i);
  } else {
    Serial.println("Unknown coin");
  }
}

void fetchCryptoPrices(int i) {
  DynamicJsonDocument doc(4096);
  if (WiFi.status() == WL_CONNECTED) {
    setupAPIForCoin(i); // Set up API for the specific coin
    http3.begin(api);
    int httpCode = http3.GET();
    if (httpCode > 0) {
      String payload = http3.getString();

      errFetch = deserializeJson(doc, payload);
      if (errFetch) {
        Serial.println("Error Fetch") ;
        handleJsonError(errFetch);
        return;
      }
      size_t heapSize = ESP.getFreeHeap();
      Serial.print("HEAP:");
      Serial.println(heapSize);
      parseCryptoPriceResponse(doc, i); // Update prices for the specific coin
    } else {
      Serial.println("Error on HTTP request");
    }
    http3.end();
  } else {
    Serial.println("connect fail");
  }
}

void updateStockPrices(DynamicJsonDocument & doc, int i) {
  if (coinType == 4) { // stock only

    JsonObject meta = doc["chart"]["result"][0]["meta"];
    String fromCurrency = meta["currency"].as<String>(); // สกุลเงินจริงจากแหล่ง API
    String toCurrency = currency[currencyIndex];         // สกุลเงินที่ผู้ใช้เลือก

    // ดึงราคาจริงจาก API
    float lastPrice = meta["regularMarketPrice"].as<float>();
    float dayHigh   = meta["regularMarketDayHigh"].as<float>();
    float dayLow    = meta["regularMarketDayLow"].as<float>();

    float openPrice = doc["chart"]["result"][0]["indicators"]["quote"][0]["open"][0].as<float>();
    float closePrice = doc["chart"]["result"][0]["indicators"]["quote"][0]["close"][0].as<float>();
    float change = closePrice - openPrice;
    float changePercent = (change / openPrice) * 100.0;

    // เก็บในหน่วย base currency (ยังไม่แปลง exRate)
    price[i]              = lastPrice;
    highPrice[i]          = dayHigh;
    lowPrice[i]           = dayLow;
    priceChange[i]        = change;
    priceChangePercent[i] = changePercent;

    updateDisplayValues(); // คูณ exRate ทีหลัง

    Serial.printf("STOCK [%s] %.2f %s → %.2f %s\n",
                  coin[i].c_str(),
                  lastPrice, fromCurrency.c_str(),
                  displayPrice[i], toCurrency.c_str());
  }
}


void fetchStockPrices(int i) {
  DynamicJsonDocument doc(4096);
  if (WiFi.status() == WL_CONNECTED) {
    setupAPIForCoin(i); // Set up API for the specific coin
    http3.begin(api);
    int httpCode = http3.GET();
    if (httpCode > 0) {
      String payload = http3.getString();

      errFetch = deserializeJson(doc, payload);
      if (errFetch) {
        handleJsonError(errFetch);
        return;
      }
      updateStockPrices(doc, i); // Update prices for the specific coin
    } else {
      Serial.println("Error on HTTP request");
    }
    http3.end();
  } else {
    Serial.println("connect fail");
  }
}

/*************************************************************
   2) แยกฟังก์ชันอัปเดตราคาดัชนีจาก JSON ของ Yahoo Finance
 *************************************************************/
void updateIndicePrices(DynamicJsonDocument &doc, int i)
{
  JsonObject result = doc["chart"]["result"][0];
  JsonObject meta   = result["meta"];

  String fromCurrency = meta["currency"].as<String>();
  String toCurrency   = currency[currencyIndex];

  // ราคาตามที่ดึงได้จากต้นฉบับ
  float lastPrice = meta["regularMarketPrice"].as<float>();
  float dayHigh   = meta["regularMarketDayHigh"].as<float>();
  float dayLow    = meta["regularMarketDayLow"].as<float>();
  float prevClose = meta["chartPreviousClose"].as<float>();

  float change    = lastPrice - prevClose;
  float changePct = (change / prevClose) * 100.0;

  // ถ้า fromCurrency เป็น THB ให้แปลงเป็น USD ก่อน
  if (fromCurrency == "THB") {
    lastPrice /= USDTHBRate;
    dayHigh   /= USDTHBRate;
    dayLow    /= USDTHBRate;
    prevClose /= USDTHBRate;
  }

  // เก็บในหน่วย base currency
  price[i]              = lastPrice;
  highPrice[i]          = dayHigh;
  lowPrice[i]           = dayLow;
  priceChange[i]        = change;
  priceChangePercent[i] = changePct;

  updateDisplayValues(); // คูณ exRate ทีหลัง

  // Debug แสดงข้อมูลหลังคูณแล้ว
  Serial.printf("INDICE [%s] %.2f %s → %.2f %s (Δ %.2f, %.2f%%)\n",
                coin[i].c_str(),
                lastPrice, fromCurrency.c_str(),
                displayPrice[i], toCurrency.c_str(),
                displayChange[i], displayChangePercent[i]);
}



/*************************************************************
   1) สร้าง API และดาวน์โหลดข้อมูลสำหรับดัชนี (coinType = 7)
 *************************************************************/
void fetchIndicePrices(int i)
{
  DynamicJsonDocument doc(4096);
  if (WiFi.status() != WL_CONNECTED) return;

  setupAPIForCoin(i);          // ได้ค่า api ถูกต้องจาก type = 7
  http3.begin(api);

  int httpCode = http3.GET();
  if (httpCode > 0)
  {
    String payload = http3.getString();
    errFetch = deserializeJson(doc, payload);
    if (errFetch)
    {
      handleJsonError(errFetch);
      http3.end();
      return;
    }
    updateIndicePrices(doc, i);  // ไปอัปเดตตัวแปรราคา
  }
  else
  {
    Serial.println("HTTP error (indice)");
  }
  http3.end();
}


void updateGERPrices(DynamicJsonDocument & doc, int i) {
  if (i >= 0 && i <= 3) {
    String currencyX = (currencyIndex == 0) ? "USD" : currency[currencyIndex];

    // ดึงข้อมูลอัตราแลกเปลี่ยนแบบ base currency
    price[i] = doc["rates"][currencyX].as<float>();
    lowPrice[i] = 0;
    highPrice[i] = 0;
    priceChange[i] = 0;
    priceChangePercent[i] = 0;
    exRate[i][currencyIndex] = 1.0;
    updateDisplayValues(); // คูณ exRate[i][currencyIndex] ทีหลัง

    // แสดงผลที่แปลงแล้ว
    Serial.print("coin: ");
    Serial.print(coin[i]);
    Serial.print(" exchange rate: ");
    Serial.println(displayPrice[i]);
  }
}

void fetchGERPrices(int i) {
  DynamicJsonDocument doc(4096);
  if (WiFi.status() == WL_CONNECTED) {
    if (coinType == 6) {
      setupAPIForCoin(i); // Set up API for the specific coin
      http3.begin(api);
      int httpCode = http3.GET();
      if (httpCode > 0) {
        String payload = http3.getString();

        errFetch = deserializeJson(doc, payload);
        if (errFetch) {
          handleJsonError(errFetch);
          return;
        }
        updateGERPrices(doc, i); // Update prices for the specific coin
      } else {
        Serial.println("Error on HTTP request");
      }
      http3.end();

    } else {
      Serial.println("connect fail");
    }
  }
}

String formatAlertPrice(float priceTemp) {
  String priceStr = "";
  String unit = "";

  if ((int)priceTemp % 1000 == 999 || (int)priceTemp % 1000 == 998) {
    priceTemp = ceil(priceTemp / 1000.0) * 1000.0;
  }

  if (priceTemp < 0.000001) {
    priceTemp *= 1e9;
    unit = "n";
  } else if (priceTemp < 0.001) {
    priceTemp *= 1e6;
    unit = "u";
  } else if (priceTemp >= 1e9) {
    priceTemp /= 1e6;
    unit = " M";
  }

  if (priceTemp >= 100000) {
    priceStr = String(priceTemp, 0);
  } else if (priceTemp >= 10000) {
    priceStr = String(priceTemp, 1);
  } else if (priceTemp >= 1000) {
    priceStr = String(priceTemp, 2);
  } else if (priceTemp >= 100) {
    priceStr = String(priceTemp, 3);
  } else if (priceTemp >= 10) {
    priceStr = String(priceTemp, 4);
  } else {
    priceStr = String(priceTemp, 5);
  }

  return priceStr + unit;
}

void checkPriceAlerts() {
  bool alertTriggered = false;
  float adaptPrice[4] = {0.0, 0.0, 0.0, 0.0};

  // // ====== เริ่มการแจ้งเตือน ======
  Serial.println("===== [Check Price Alerts] =====");
  Serial.print("📌 currencyAlert: ");
  Serial.println(currencyAlert);
  Serial.print("📌 currency1: ");
  Serial.println(currency1);
  Serial.print("📌 currency2: ");
  Serial.println(currency2);
  Serial.print("📌 currencyIndex: ");
  Serial.println(currencyIndex);

  int alertIndex = (currencyAlert == currency2) ? ((currencyIndex == 0) ? 1 : 0) : currencyIndex;

  Serial.print("📌 alertIndex (currency used for alert): ");
  Serial.println(alertIndex);

  for (int i = 0; i < 4; i++) {
    if (currencyAlert == currency1 && currencyIndex == 0)
    {
      adaptPrice[i] = displayPrice[i];
    }
    else if (currencyAlert == currency1 && currencyIndex == 1)
    {
      adaptPrice[i] = displayPrice[i] / exRate[i][alertIndex];;
    }
    else if (currencyAlert == currency2 && currencyIndex == 0)
    {
      adaptPrice[i] = displayPrice[i] * exRate[i][alertIndex];;
    }
    else if (currencyAlert == currency2 && currencyIndex == 1)
    {
      adaptPrice[i] = displayPrice[i];
    }

    Serial.printf("---- Coin[%d]: %s ----\n", i, coin[i].c_str());
    Serial.printf("🪙 Base Price: %.8f\n", price[i]);
    Serial.printf("💱 ExRate[%d][%d]: %.8f\n", i, alertIndex, exRate[i][alertIndex]);
    Serial.printf("📈 Adapted Price (%s): %.8f\n", currencyAlert.c_str(), adaptPrice[i]);

    for (int j = 0; j < 8; j++) {
      float low = coinLow[i][j];
      float high = coinHigh[i][j];

      // ข้ามถ้าค่าที่ตั้งเป็น 0
      if (low == 0 && high == 0) continue;

      // ข้ามถ้าราคาห่างจากค่าที่ตั้งเกิน 1000 เท่า
      if ((low > 0 && adaptPrice[i] / low > 1000) || (high > 0 && high / adaptPrice[i] > 1000)) {
        continue;
      }

      // // Serial.printf("  ✅ Check alert pair %d:\n", j);
      // // Serial.printf("    - Low Enabled: %s, Threshold: %.8f\n", coinLowStatus[i][j] ? "true" : "false", low);
      // // Serial.printf("    - High Enabled: %s, Threshold: %.8f\n", coinHighStatus[i][j] ? "true" : "false", high);

      if (coinLowStatus[i][j] && adaptPrice[i] < low) {
        // // Serial.println("🚨 LOW ALERT Triggered!");
        alertCoin = coin[i];
        alertPrice = formatAlertPrice(adaptPrice[i]);
        alertStatus = "LOW";
        alertTriggered = true;
        alertI = i;
        alertJ = j;
        break;
      } else if (coinHighStatus[i][j] && adaptPrice[i] > high) {
        // // Serial.println("🚨 HIGH ALERT Triggered!");
        alertCoin = coin[i];
        alertPrice = formatAlertPrice(adaptPrice[i]);
        alertStatus = "HIGH";
        alertTriggered = true;
        alertI = i;
        alertJ = j;
        break;
      }
    }
  }

  if (alertTriggered) {
    // // Serial.println("✅ Alert triggered → pageIndex = 22");
    pageIndex = 22;
  } else {
    // // Serial.println("⛔️ No alerts triggered.");
  }

  // // Serial.println("===============================");
}




// Function to reset coin values to default when not fetched
void resetCoinValues(int i) {
  int alertIndex = (currencyAlert == currency2) ? ((currencyIndex == 0) ? 1 : 0) : currencyIndex;

  for (int j = 0; j < 8; j++) {
    coinLow[i][j] = 0.0;
    coinHigh[i][j] = 10000000.0 / exRate[i][alertIndex];
    coinLowStatus[i][j] = false;
    coinHighStatus[i][j] = false;
  }
}


void fetchAndDisplayPrice() {
  Serial.print("Fetch : ");
  Serial.print(coin[coinIndex]);
  Serial.print("/");
  Serial.println(currency[currencyIndex]);

  waitforprice = true;

  for (int i = 0; i < 4; i++) {
    size_t heapSize = ESP.getFreeHeap();
    Serial.print("HEAP");
    Serial.print(i);
    Serial.print(":");
    Serial.println(heapSize);

    coinType = type[i].toInt();
    if (i != coinIndex) {
      // do nothing
    }
    else if (i == coinIndex)
    {
      if (coinType == 0) {
        if (coin[i] == "GOLD") {
          Serial.println("Fetch Gold1");
          fetchGoldPrices();
        } else if (coin[i] == "JBC") {
          fetchCryptoPrices(i);
        } else if (coin[i] == "XAU") {
          fetchCryptoPrices(i);
        }
      }
      if (coinType == 1 || coinType == 2 || coinType == 3) {
        fetchCryptoPrices(i);
      }
      else if (coinType == 4) {
        fetchStockPrices(i);
      }
      else if (coinType == 5) {
        // blank
      }
      else if (coinType == 6) {
        if (coin[i] == "USD") {
          currencyIndex = 1;
          Serial.println("🔄 coinType 6: USD → สลับ currency เป็น THB ทันที");
        }

        if (coin[i] == currency[currencyIndex] || (coin[i] == "USD" && currencyIndex == 0)) {
          price[i] = 1.0;
          lowPrice[i] = 0;
          highPrice[i] = 0;
          priceChange[i] = 0;
          priceChangePercent[i] = 0;
        } else {
          fetchGERPrices(i);
        }
      }
      else if (coinType == 7) {
        fetchIndicePrices(i);
      }
    }
  }

  coinType = type[coinIndex].toInt();
  if (alertEnable == true && pauseAlert == false)
  {
    // Check for price alerts on the selected coin
    checkPriceAlerts();
  }
  waitforprice = false;

  updateDisplayValues(); // 🔴 เพิ่มตรงนี้ เพื่อคูณ exRate หลังดึงข้อมูลเสร็จ

  lastUpdateTime = millis() - updateInterval;
}


// ภายใน Task ใช้ Global Parameter ในการ delay
void fetchAndDisplayPriceTask(void *parameter) {
  Serial.println("pageIndex:");
  Serial.println(pageIndex);
  if (pageIndex == 2)
  {
    for (;;) {
      Serial.println("Update durationTask !!!");
      durationTask = millis();
      durationTaskCount = 0  ;
      fetchAndDisplayPrice();
      // ใช้ Global Parameter ในการ delay
      vTaskDelay(pdMS_TO_TICKS(updateInterval));

    }
    vTaskDelete(NULL);
  }
  else
  {
    Serial.println("Error fetch page: ");
    Serial.println(pageIndex);
  }
}


void gotopage2() {
  pageIndex = 2;

  silenceBuzzer();
  save_config() ;
  tft.fillScreen(TFT_BLACK); // Clear the screen
  tft.setTextColor(TFT_WHITE);
  if (coin[coinIndex] == "GOLD")
  {
    last_bar_sell = 0 ;
    last_bar_buy = 0 ;
    last_jewelry_sell = 0 ;
    last_jewelry_buy = 0 ;

    displayGoldOnScreen();
  }
  else //normal crypo
  {
    displayBackgroundImageForPage2();
    showLogoAtTopMiddle();
    showLogoAtBottomMiddle();
    //displayCoinOnScreen();
    showPage2();
  }
  silenceBuzzer();
  delay(500);
  startPriceTask();
  silenceBuzzer();

}

void adjustBrightness() {
  int pwmValue = map(brightnessLevel, 0, 100, 0, 255);
  analogWrite(21, pwmValue);

}

void increaseBrightness() {
  if (brightnessLevel < 10) {
    brightnessLevel += 2; // ถ้าต่ำกว่า 10 ให้เพิ่มทีละ 2
    if (brightnessLevel > 10) {
      brightnessLevel = 10; // จำกัดไม่ให้เกิน 10
    }
  } 
  else if (brightnessLevel < 100) {
    brightnessLevel += 30; // ถ้ามากกว่าเท่ากับ 10 แล้ว เพิ่มทีละ 30
    if (brightnessLevel > 100) {
      brightnessLevel = 100; // จำกัดสูงสุดที่ 100
    }
  }

  adjustBrightness();           // ปรับความสว่างใหม่
  tft.fillScreen(TFT_BLACK);    // เคลียร์หน้าจอ
  showPage3();                  // แสดงหน้าเพจ 3
  delay(1500);
  //loadSettingsAndPrintToSerial();
}


void decreaseBrightness() {
  if (brightnessLevel > 30) {
    brightnessLevel -= 30; // ลดความสว่างลง 30
    if (brightnessLevel < 30 && brightnessLevel > 10) {
      brightnessLevel = 10; // หากต่ำกว่า 30 แต่ยังมากกว่า 10 ให้ตั้งไว้ที่ 10
    }
  } else if (brightnessLevel <= 10 && brightnessLevel > 0) {
    brightnessLevel -= 2; // เมื่อถึง 10 หรือน้อยกว่านั้น ลดลงครั้งละ 2
    if (brightnessLevel <= 0) {
      brightnessLevel = 2; // ไม่ให้ต่ำกว่า 0
    }
  }

  adjustBrightness();           // ปรับความสว่างใหม่
  tft.fillScreen(TFT_BLACK);    // เคลียร์หน้าจอ
  showPage3();                  // แสดงหน้าเพจ 3
  delay(1500);
  //loadSettingsAndPrintToSerial();
}


void increaseInterval() {
  if (currentIntervalIndex < 4) {
    currentIntervalIndex++;
    updateInterval = intervals[currentIntervalIndex] * 1000; // Convert to milliseconds
    // Update any relevant display or settings here
    save_config() ; // Remember to update your settings storage
    tft.fillScreen(TFT_BLACK); // Clear the screen
    showPage3();
    delay(1500);
  }
}

void decreaseInterval() {
  if (currentIntervalIndex > 0) {
    currentIntervalIndex--;
    updateInterval = intervals[currentIntervalIndex] * 1000; // Convert to milliseconds
    // Update any relevant display or settings here
    save_config() ;
    tft.fillScreen(TFT_BLACK); // Clear the screen
    showPage3();
    delay(1500);
  }
}

void increaseTimeout() {
  if (currentTimeoutIndex < 4) {
    currentTimeoutIndex++;
    timeout = timeouts[currentTimeoutIndex] * 1000; // Convert to milliseconds
    // Update any relevant display or settings here
    save_config() ;
    tft.fillScreen(TFT_BLACK); // Clear the screen
    showPage3();
    delay(1500);
  }
}

void decreaseTimeout() {
  if (currentTimeoutIndex > 0) {
    currentTimeoutIndex--;
    timeout = timeouts[currentTimeoutIndex] * 1000; // Convert to milliseconds
    // Update any relevant display or settings here
    save_config() ;
    tft.fillScreen(TFT_BLACK); // Clear the screen
    showPage3();
    delay(1500);
  }
}

void gotopage1() {
  Serial.println("Go to page 1");
  pageUpdate = true;
  pageIndex = 1;
  stopPriceTask();
  save_config() ;
  tft.fillScreen(TFT_BLACK); // Clear the screen
  showPage1();
  delay(50);
}

void drawGradientRoundedRect(int x, int y, int w, int h, int radius, uint32_t colorStart, uint32_t colorEnd) {
  float delta = 255.0 / h;
  float alpha = 0.0;

  for (int i = 0; i < h; i++) {
    uint32_t currentColor = fastBlend((uint8_t)alpha, colorStart, colorEnd);

    if (i < radius || i >= h - radius) {
      int len = sqrt(radius * radius - pow(radius - (i < radius ? i : h - 1 - i), 2));
      tft.drawFastHLine(x + radius - len, y + i, w - 2 * (radius - len), currentColor);
    } else {
      tft.drawFastHLine(x, y + i, w, currentColor);
    }

    alpha += delta;
  }


}

uint32_t fastBlend(uint8_t alpha, uint32_t fgc, uint32_t bgc) {
  uint32_t rxb = bgc & 0xF81F;
  rxb += ((fgc & 0xF81F) - rxb) * (alpha >> 2) >> 6;
  uint32_t xgx = bgc & 0x07E0;
  xgx += ((fgc & 0x07E0) - xgx) * alpha >> 8;
  return (rxb & 0xF81F) | (xgx & 0x07E0);
}

void drawArrow(int startX, int startY, String direction) {
  int arrowWidth = 20;  // Width of the arrow
  int arrowHeight = 30; // Height of the arrow

  if (direction == "left") {
    // Left Arrow
    tft.fillRect(startX, startY + 10, arrowWidth, arrowHeight - 20, PASTEL_ARROW);
    tft.fillTriangle(
      startX - 10, startY + (arrowHeight / 2), // Left point (tip of the arrow)
      startX, startY + 5, // Right top point
      startX, startY + arrowHeight - 5, // Right bottom point
      PASTEL_ARROW);
  } else if (direction == "right") {
    // Right Arrow
    tft.fillRect(startX - arrowWidth, startY + 10, arrowWidth, arrowHeight - 20, PASTEL_ARROW);
    tft.fillTriangle(
      startX + 10, startY + (arrowHeight / 2), // Right point (tip of the arrow)
      startX, startY + 5, // Left top point
      startX, startY + arrowHeight - 5, // Left bottom point
      PASTEL_ARROW);
  } else if (direction == "up") {
    // Up Arrow
    tft.fillRect(startX - 6, startY + 18, arrowWidth - 7, arrowHeight - 10, PASTEL_ARROW);
    tft.fillTriangle(
      startX, startY, // Top point (tip of the arrow)
      startX - arrowWidth / 2, startY + arrowHeight - 13, // Bottom left point
      startX + arrowWidth / 2, startY + arrowHeight - 13, // Bottom right point
      PASTEL_ARROW);
  } else if (direction == "down") {
    // Down Arrow
    tft.fillRect(startX - 6, startY - arrowHeight - 7, arrowWidth - 7, arrowHeight - 10, PASTEL_ARROW);
    tft.fillTriangle(
      startX, startY, // Bottom point (tip of the arrow)
      startX - arrowWidth / 2, startY - arrowHeight + 13, // Top left point
      startX + arrowWidth / 2, startY - arrowHeight + 13, // Top right point
      PASTEL_ARROW);
  }
}

// Function to draw a circle with text
void drawCircleWithText(int x, int y, int radius, uint16_t fillColor, uint16_t borderColor, uint16_t txtColor, const char* text) {
  tft.fillCircle(x, y, radius, fillColor);
  tft.drawCircle(x, y, radius, borderColor);
  tft.setTextDatum(MC_DATUM);
  tft.setTextColor(txtColor);
  tft.drawString(text, x, y);
}

// Function to draw keypad
void drawKeypad() {
  tft.fillScreen(TFT_BLACK);

  // Draw the big text box
  tft.fillRoundRect(5, 10, 293, 50, 10, TFT_WHITE); // Rounded corners with radius 10
  tft.setTextColor(TFT_BLACK, TFT_WHITE);
  tft.setTextDatum(MC_DATUM);
  tft.setTextSize(3);
  tft.drawString(inputText, 155, 35); // Centered in the box

  // Define pastel colors
  uint16_t pastelColors[] = {TFT_PINK, TFT_CYAN, TFT_LIGHTGREY, TFT_ORANGE, TFT_GREEN};

  // Draw number keys 0-9
  for (int i = 0; i < 10; i++) {
    int x = keyStartX + (i % 5) * (keyWidth + keyPadding);
    int y = keyStartY + (i / 5) * (keyHeight + keyPadding);

    // Use pastel colors in a loop
    uint16_t keyColor = pastelColors[i % 5];
    tft.fillRoundRect(x, y, keyWidth, keyHeight, 10, keyColor); // Rounded corners with radius 10
    tft.setTextColor(TFT_BLACK, keyColor);
    tft.setTextSize(2);
    tft.drawString(String(i), x + keyWidth / 2, y + keyHeight / 2);
  }

  drawArrow(25, 195, "left");

  // Draw . key
  int i = 12;
  int x = keyStartX + (i % 5) * (keyWidth + keyPadding);
  int y = keyStartY + (i / 5) * (keyHeight + keyPadding);

  tft.fillRoundRect(x, y, keyWidth, keyHeight, 10, TFT_LIGHTGREY);
  tft.setTextColor(TFT_BLACK, TFT_LIGHTGREY);
  tft.setTextSize(2);
  tft.drawString(".", x + keyWidth / 2, y + keyHeight / 2);

  // Draw D, C, OK buttons with rounded edges and pastel colors
  i = 11;
  x = keyStartX + (i % 5) * (keyWidth + keyPadding);
  y = keyStartY + (i / 5) * (keyHeight + keyPadding);
  drawCircleWithText(x + 25, y + 25, 25, TFT_PINK, TFT_WHITE, TFT_WHITE, "D");

  i = 13;
  x = keyStartX + (i % 5) * (keyWidth + keyPadding);
  y = keyStartY + (i / 5) * (keyHeight + keyPadding);
  drawCircleWithText(x + 25, y + 25, 25, TFT_CYAN, TFT_WHITE, TFT_WHITE, "C");

  i = 14;
  x = keyStartX + (i % 5) * (keyWidth + keyPadding);
  y = keyStartY + (i / 5) * (keyHeight + keyPadding);
  drawCircleWithText(x + 25, y + 23, 25, TFT_GREEN, TFT_WHITE, TFT_WHITE, "OK");
}


void drawQRCode(const char *url, int type , int  scale , int ydiff)
{
  QRCode qrcode;
  uint8_t qrcodeData[qrcode_getBufferSize(type)];
  qrcode_initText(&qrcode, qrcodeData, type, 0, url);


  int border = scale * 1; // White border size, effectively 4 modules wide

  // Calculate the total QR code size excluding the border
  int qrCodeSize = qrcode.size * scale;

  // Calculate the offset to center the QR code on the screen
  int xOffset = (tft.width() - qrCodeSize) / 2;
  int yOffset = (tft.height() - qrCodeSize) / 2 + ydiff;

  // Adjust offsets for the border by subtracting border size to start drawing from a point that includes the border in calculation
  int borderedXOffset = xOffset - border;
  int borderedYOffset = yOffset - border;

  // Calculate total size including the border
  int totalSize = qrCodeSize + border * 2;

  // Draw a white rectangle for the border around the QR code on the black background
  tft.fillRect(borderedXOffset, borderedYOffset, totalSize, totalSize, TFT_WHITE);
  // Display QR code with the specified scale and offset, considering the border.
  // Now the QR code starts drawing from the original xOffset and yOffset without the border adjustment
  for (int y = 0; y < qrcode.size; y++) {
    for (int x = 0; x < qrcode.size; x++) {
      if (qrcode_getModule(&qrcode, x, y)) {
        tft.fillRect(x * scale + xOffset, y * scale + yOffset, scale, scale, TFT_BLACK);
      }
      // No need to draw white squares for modules because the entire QR code area inside the border is initially white
    }
  }
}

uint16_t rgbTo565(uint8_t r, uint8_t g, uint8_t b) {
  return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
}

void silenceBuzzer() {
  ledcDetachPin(BUZZER);
  pinMode(BUZZER, OUTPUT);
  digitalWrite(BUZZER, LOW);
}



// ฟังก์ชันปรับเสียงเบาลงแบบ PWM (ทำด้วยซอฟต์แวร์)
void buzz_soft(long frequency, long length) {
  int pwmPin = BUZZER;
  
  // เนื่องจากฮาร์ดแวร์ Buzzer น่าจะเป็นแบบจ่ายไฟปุ๊บดังสุดทันที (Active Buzzer หรือมี Transistor ขยาย)
  // วิธีลดเสียงที่ได้ผลที่สุดคือ "ลดเวลาที่เปิดเสียงให้สั้นมากๆ" กลายเป็นเสียง "จิ๊บ" แทน "บี๊บ"
  digitalWrite(pwmPin, HIGH); 
  delay(2); // เปิดเสียงแค่ 2 มิลลิวินาที (สั้นมากๆ)
  digitalWrite(pwmPin, LOW);
  
  if (length > 2) {
    delay(length - 2); // รอเวลาที่เหลือให้ครบ length
  }
  silenceBuzzer();
}

// ฟังก์ชันสำหรับเสียงปี๊บเบาๆ สั้นๆ 1 ครั้ง ทุกๆ 1 วินาที
void beepThreeTimes() {
  static unsigned long lastBeepTime = 0;
  
  // เช็คว่าผ่านไป 1 วินาที (1000 ms) หรือยัง
  if (millis() - lastBeepTime >= 1000) {
    
    // สร้างคลื่นความถี่สำหรับ Passive Buzzer ด้วยซอฟต์แวร์ (2000 Hz)
    // ระยะเวลาสั้นมากๆ แค่ 15 มิลลิวินาที ทำให้เสียงที่ได้เป็น "ปี๊บ" เบาๆ และสั้นมาก
    int pwmPin = BUZZER;
    long frequency = 2000; 
    long period = 1000000 / frequency; // 500 ไมโครวินาทีต่อรอบ
    long numCycles = 30; // 30 รอบ * 500us = 15 มิลลิวินาที
    
    for (long i = 0; i < numCycles; i++) {
      digitalWrite(pwmPin, HIGH); 
      delayMicroseconds(20); // กระตุ้นสั้นๆ
      digitalWrite(pwmPin, LOW);
      delayMicroseconds(period - 20);
    }
    
    lastBeepTime = millis();
  }
}
