#include "BitkubApi.h"
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

static const char BITKUB_BASE_URL[] = "https://api.bitkub.com";

static bool beginSecureRequest(HTTPClient& http, WiFiClientSecure& client, const String& url) {
  if (WiFi.status() != WL_CONNECTED) return false;
  client.setInsecure();  // ตัวอย่างเพื่อการสอน: HTTPS แต่ไม่ได้ฝัง CA certificate
  http.setTimeout(12000);
  http.setUserAgent("CryptoClock-Learning/1.0");
  return http.begin(client, url);
}

bool fetchBitkubTicker(Coin& coin) {
  WiFiClientSecure client;
  HTTPClient http;
  String url = String(BITKUB_BASE_URL) + "/api/v3/market/ticker?sym=" + coin.apiSymbol;

  if (!beginSecureRequest(http, client, url)) return false;
  int httpCode = http.GET();
  if (httpCode != HTTP_CODE_OK) {
    Serial.printf("Ticker %s failed: HTTP %d\n", coin.apiSymbol, httpCode);
    http.end();
    return false;
  }

  DynamicJsonDocument doc(1536);
  DeserializationError jsonError = deserializeJson(doc, http.getStream());
  http.end();
  if (jsonError || !doc.is<JsonArray>() || doc.size() == 0) {
    Serial.printf("Ticker %s JSON error\n", coin.apiSymbol);
    return false;
  }

  JsonObject ticker = doc[0];
  coin.lastPrice = ticker["last"].as<float>();
  coin.high24h = ticker["high_24_hr"].as<float>();
  coin.low24h = ticker["low_24_hr"].as<float>();
  coin.changePercent = ticker["percent_change"].as<float>();
  coin.hasData = coin.lastPrice > 0;

  Serial.printf("%s = %.2f THB\n", coin.symbol, coin.lastPrice);
  return coin.hasData;
}

void fetchAllBitkubTickers() {
  for (size_t i = 0; i < COIN_COUNT; i++) {
    fetchBitkubTicker(coins[i]);
    delay(80);
  }
}

static uint32_t getBitkubUnixTime() {
  WiFiClientSecure client;
  HTTPClient http;
  String url = String(BITKUB_BASE_URL) + "/api/v3/servertime";
  if (!beginSecureRequest(http, client, url)) return 0;

  int httpCode = http.GET();
  if (httpCode != HTTP_CODE_OK) {
    http.end();
    return 0;
  }

  String payload = http.getString();
  http.end();
  uint64_t milliseconds = strtoull(payload.c_str(), nullptr, 10);
  return static_cast<uint32_t>(milliseconds / 1000ULL);
}

static void calculateCdc(CdcData& data) {
  if (data.count == 0) return;

  data.emaFast[0] = data.candles[0].close;
  data.emaSlow[0] = data.candles[0].close;
  const float fastFactor = 2.0f / (CDC_EMA_FAST + 1.0f);
  const float slowFactor = 2.0f / (CDC_EMA_SLOW + 1.0f);

  for (size_t i = 1; i < data.count; i++) {
    float close = data.candles[i].close;
    data.emaFast[i] = close * fastFactor + data.emaFast[i - 1] * (1.0f - fastFactor);
    data.emaSlow[i] = close * slowFactor + data.emaSlow[i - 1] * (1.0f - slowFactor);
  }

  size_t last = data.count - 1;
  data.buyZone = data.emaFast[last] > data.emaSlow[last];
  data.hasData = true;
}

bool fetchBitkubCdcHistory(CdcData& data) {
  data.error = "";
  if (WiFi.status() != WL_CONNECTED) {
    data.error = "WiFi disconnected";
    return false;
  }

  uint32_t toTime = getBitkubUnixTime();
  if (toTime == 0) {
    data.error = "Cannot read server time";
    return false;
  }

  const uint32_t secondsPerCandle = 4UL * 60UL * 60UL;
  uint32_t fromTime = toTime - (secondsPerCandle * CDC_CANDLE_COUNT);
  String url = String(BITKUB_BASE_URL) + "/tradingview/history?symbol=" + CDC_SYMBOL;
  url += "&resolution=" + String(CDC_RESOLUTION);
  url += "&from=" + String(fromTime);
  url += "&to=" + String(toTime);

  WiFiClientSecure client;
  HTTPClient http;
  if (!beginSecureRequest(http, client, url)) {
    data.error = "Cannot start HTTP";
    return false;
  }

  int httpCode = http.GET();
  if (httpCode != HTTP_CODE_OK) {
    data.error = "HTTP " + String(httpCode);
    http.end();
    return false;
  }

  DynamicJsonDocument doc(24576);
  DeserializationError jsonError = deserializeJson(doc, http.getStream());
  http.end();
  if (jsonError || String(doc["s"] | "") != "ok") {
    data.error = jsonError ? jsonError.c_str() : "History unavailable";
    return false;
  }

  JsonArray opens = doc["o"].as<JsonArray>();
  JsonArray highs = doc["h"].as<JsonArray>();
  JsonArray lows = doc["l"].as<JsonArray>();
  JsonArray closes = doc["c"].as<JsonArray>();
  size_t count = opens.size();
  if (highs.size() < count) count = highs.size();
  if (lows.size() < count) count = lows.size();
  if (closes.size() < count) count = closes.size();
  if (count > CDC_CANDLE_COUNT) count = CDC_CANDLE_COUNT;
  if (count == 0) {
    data.error = "No candle data";
    return false;
  }

  data.count = count;
  for (size_t i = 0; i < count; i++) {
    data.candles[i] = {
      opens[i].as<float>(), highs[i].as<float>(),
      lows[i].as<float>(), closes[i].as<float>()
    };
  }

  calculateCdc(data);
  return true;
}
