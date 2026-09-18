#include <Arduino.h>
#include <TFT_eSPI.h>
#include <WiFi.h>
#include <HTTPClient.h>

#include "GlobalModule.h"
#include "GlobalParameter.h"
#include "JpgSupport.h"
#include "page26.h"

#define CDC_PAGE_KLINE_COUNT 100
#define CDC_PAGE_EMA_FAST 12
#define CDC_PAGE_EMA_SLOW 26

struct CDCKlineData {
  float open;
  float high;
  float low;
  float close;
};

struct CDCPageResult {
  float emaFast;
  float emaSlow;
  int8_t zone;
  int8_t signal;
};

static const char* CDC_PAGE_SYMBOL = "BTCUSDT";
static const char* CDC_PAGE_INTERVALS[] = {"1h", "4h", "1d"};
static const char* CDC_PAGE_INTERVAL_LABELS[] = {"1H", "4H", "1D"};
static const int CDC_PAGE_INTERVAL_COUNT = sizeof(CDC_PAGE_INTERVALS) / sizeof(CDC_PAGE_INTERVALS[0]);
static int cdcPageIntervalIndex = 1;

static const int CDC_TF_BUTTON_W = 40;
static const int CDC_TF_BUTTON_H = 18;
static const int CDC_TF_TOUCH_PAD = 14;

static const char* currentCDCPageInterval() {
  return CDC_PAGE_INTERVALS[cdcPageIntervalIndex];
}

static const char* currentCDCPageIntervalLabel() {
  return CDC_PAGE_INTERVAL_LABELS[cdcPageIntervalIndex];
}

static int cdcTFButtonX() {
  const int badgeW = 82;
  int badgeX = tft.width() - badgeW;
  return badgeX + ((badgeW - CDC_TF_BUTTON_W) / 2);
}

static int cdcTFButtonY() {
  return 81;
}

static bool isCDCIntervalButtonHit(int x, int y) {
  int buttonX = cdcTFButtonX();
  int buttonY = cdcTFButtonY();
  return x >= buttonX - CDC_TF_TOUCH_PAD &&
         x <= buttonX + CDC_TF_BUTTON_W + CDC_TF_TOUCH_PAD &&
         y >= buttonY - CDC_TF_TOUCH_PAD &&
         y <= buttonY + CDC_TF_BUTTON_H + CDC_TF_TOUCH_PAD;
}

static void commitKlineValue(CDCKlineData &kline, int elementIndex, const String &value) {
  if (value.length() == 0) return;

  if (elementIndex == 1) {
    kline.open = value.toFloat();
  } else if (elementIndex == 2) {
    kline.high = value.toFloat();
  } else if (elementIndex == 3) {
    kline.low = value.toFloat();
  } else if (elementIndex == 4) {
    kline.close = value.toFloat();
  }
}

static bool parseBTCKlineStream(HTTPClient &http, CDCKlineData klines[], int &count, String &outError) {
  count = 0;
  WiFiClient *stream = http.getStreamPtr();
  int bracketLevel = 0;
  int elementIndex = 0;
  String currentValue = "";
  bool inQuotes = false;
  uint32_t lastDataAt = millis();

  while (http.connected() || stream->available()) {
    if (!stream->available()) {
      if (millis() - lastDataAt > 15000) {
        outError = "Stream timeout";
        Serial.println(outError);
        break;
      }
      delay(1);
      continue;
    }

    lastDataAt = millis();
    char c = stream->read();

    if (c == '"') {
      inQuotes = !inQuotes;
      continue;
    }

    if (inQuotes) {
      currentValue += c;
      continue;
    }

    if (c == '[') {
      bracketLevel++;
      if (bracketLevel == 2) {
        elementIndex = 0;
        currentValue = "";
        if (count < CDC_PAGE_KLINE_COUNT) {
          klines[count].open = 0;
          klines[count].high = 0;
          klines[count].low = 0;
          klines[count].close = 0;
        }
      }
    } else if (c == ']') {
      if (bracketLevel == 2) {
        if (count < CDC_PAGE_KLINE_COUNT) {
          commitKlineValue(klines[count], elementIndex, currentValue);
          count++;
        }
        currentValue = "";
        if (count >= CDC_PAGE_KLINE_COUNT) break;
      }
      bracketLevel--;
    } else if (c == ',') {
      if (bracketLevel == 2) {
        if (count < CDC_PAGE_KLINE_COUNT) {
          commitKlineValue(klines[count], elementIndex, currentValue);
        }
        elementIndex++;
        currentValue = "";
      }
    } else if (bracketLevel == 2 && c != ' ' && c != '\n' && c != '\r') {
      currentValue += c;
    }
  }

  if (count == 0 && outError.length() == 0) {
    outError = "No data";
  }
  return count > 0;
}

static void fetchBTCKlineData(CDCKlineData klines[], int &count, int &outHttpCode, String &outError) {
  count = 0;
  outHttpCode = 0;
  outError = "";

  if (WiFi.status() != WL_CONNECTED) {
    outError = "WiFi not connected";
    Serial.println(outError);
    return;
  }

  const char* endpointHosts[] = {
    "http://data-api.binance.vision",
    "https://data-api.binance.vision",
    "https://api1.binance.com",
    "https://api2.binance.com",
    "https://api3.binance.com",
    "https://api4.binance.com",
    "https://api.binance.com"
  };
  const int endpointCount = sizeof(endpointHosts) / sizeof(endpointHosts[0]);

  for (int i = 0; i < endpointCount; i++) {
    HTTPClient http;
    String apiUrl = String(endpointHosts[i]) + "/api/v3/klines?symbol=";
    apiUrl += CDC_PAGE_SYMBOL;
    apiUrl += "&interval=";
    apiUrl += currentCDCPageInterval();
    apiUrl += "&limit=";
    apiUrl += String(CDC_PAGE_KLINE_COUNT);

    Serial.print("Fetching BTC CDC Klines: ");
    Serial.println(apiUrl);

    if (!http.begin(apiUrl)) {
      outHttpCode = 0;
      outError = "HTTP begin failed";
      Serial.println(outError);
      delay(200);
      continue;
    }

    http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
    http.setTimeout(15000);
    http.setUserAgent("CryptoClock/4.1.4");
    http.addHeader("Accept-Encoding", "identity");
    outHttpCode = http.GET();

    if (outHttpCode == HTTP_CODE_OK) {
      outError = "";
      bool ok = parseBTCKlineStream(http, klines, count, outError);
      http.end();
      if (ok) return;

      Serial.print("BTC CDC parse failed: ");
      Serial.println(outError);
      delay(200);
      continue;
    }

    String httpError = http.errorToString(outHttpCode);
    outError = String("HTTP ") + String(outHttpCode);
    if (httpError.length() > 0) {
      outError += " ";
      outError += httpError;
    }
    Serial.print("BTC CDC HTTP GET failed: ");
    Serial.print(outHttpCode);
    Serial.print(" ");
    Serial.println(httpError);
    http.end();
    delay(200);
  }
}

static float calculateEMA(float currentPrice, float previousEMA, int period) {
  float k = 2.0 / (period + 1);
  return (currentPrice * k) + (previousEMA * (1 - k));
}

static void calculateCDC(const CDCKlineData klines[], int count, CDCPageResult results[]) {
  if (count == 0) return;

  results[0].emaFast = klines[0].close;
  results[0].emaSlow = klines[0].close;
  results[0].zone = (results[0].emaFast > results[0].emaSlow) ? 1 : -1;
  results[0].signal = 0;

  for (int i = 1; i < count; i++) {
    results[i].emaFast = calculateEMA(klines[i].close, results[i - 1].emaFast, CDC_PAGE_EMA_FAST);
    results[i].emaSlow = calculateEMA(klines[i].close, results[i - 1].emaSlow, CDC_PAGE_EMA_SLOW);

    int8_t currentZone = (results[i].emaFast > results[i].emaSlow) ? 1 : -1;
    results[i].zone = currentZone;

    if (results[i - 1].zone == -1 && currentZone == 1) {
      results[i].signal = 1;
    } else if (results[i - 1].zone == 1 && currentZone == -1) {
      results[i].signal = -1;
    } else {
      results[i].signal = 0;
    }
  }
}

static String formatBTCPrice(float price) {
  if (price >= 1000) {
    return String(price, 2);
  }
  return String(price, 4);
}

static int priceToY(float value, float minVal, float range, int chartBottom, int chartHeight) {
  int y = chartBottom - ((value - minVal) / range) * chartHeight;
  int chartTop = chartBottom - chartHeight;
  if (y < chartTop) return chartTop;
  if (y > chartBottom) return chartBottom;
  return y;
}

static void drawCDCHeader(int8_t zone, float latestClose) {
  tft.fillRect(0, 0, 320, 115, TFT_BLACK);

  int badgeW = 82;
  int badgeH = 82;
  int badgeY = 0;
  int badgeX = tft.width() - badgeW;

  tft.setTextSize(2);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setCursor(8, 7);
  tft.print("CDC ActionZone 3.0");

  if (zone == 1) {
    tft.fillRoundRect(badgeX, badgeY, badgeW, badgeH, 8, TFT_GREEN);
    tft.setTextColor(TFT_BLACK);
    tft.setTextSize(2);
    tft.setCursor(badgeX + 24, badgeY + 32);
    tft.print("BUY");
  } else {
    tft.fillRoundRect(badgeX, badgeY, badgeW, badgeH, 8, TFT_RED);
    tft.setTextColor(TFT_WHITE);
    tft.setTextSize(2);
    tft.setCursor(badgeX + 18, badgeY + 32);
    tft.print("SELL");
  }

  int tfX = cdcTFButtonX();
  int tfY = cdcTFButtonY();
  tft.fillRoundRect(tfX, tfY, CDC_TF_BUTTON_W, CDC_TF_BUTTON_H, 4, TFT_BLACK);
  tft.drawRoundRect(tfX, tfY, CDC_TF_BUTTON_W, CDC_TF_BUTTON_H, 4, TFT_WHITE);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextSize(1);
  tft.setCursor(tfX + 13, tfY + 5);
  tft.print(currentCDCPageIntervalLabel());

  String priceStr = formatBTCPrice(latestClose);
  int priceWidth = priceStr.length() * 18;
  int logoX = 10;
  int logoY = 34;

  String logoPath = "/" + firmwareVersion + "/system/image/coin1.jpg";
  tft.fillRect(logoX, logoY, 48, 48, TFT_BLACK);
  drawSdJpeg(logoPath.c_str(), logoX, logoY);

  tft.setTextSize(3);
  tft.setTextColor(tft.color565(247, 147, 26));
  int priceX = logoX + 58;
  int priceY = logoY + 13;
  if (priceX + priceWidth > badgeX - 4) {
    priceX = badgeX - priceWidth - 4;
  }
  tft.setCursor(priceX, priceY);
  tft.print(priceStr);

  tft.setTextDatum(TL_DATUM);
}

static void drawCDCChart(const CDCKlineData klines[], const CDCPageResult results[], int count) {
  if (count == 0) return;

  const int startX = 10;
  const int chartBottom = 232;
  const int chartWidth = 300;
  const int chartHeight = 126;
  const int chartTop = chartBottom - chartHeight;

  float minVal = klines[0].low;
  float maxVal = klines[0].high;

  for (int i = 0; i < count; i++) {
    if (klines[i].low < minVal) minVal = klines[i].low;
    if (klines[i].high > maxVal) maxVal = klines[i].high;
  }

  float range = maxVal - minVal;
  if (range <= 0) range = 1;
  minVal -= range * 0.05;
  maxVal += range * 0.05;
  range = maxVal - minVal;

  int barWidth = (chartWidth / CDC_PAGE_KLINE_COUNT) - 1;
  if (barWidth < 2) barWidth = 2;

  for (int i = 0; i < count; i++) {
    int x = startX + (i * (barWidth + 1));
    if (x >= startX + chartWidth) break;

    int openY = priceToY(klines[i].open, minVal, range, chartBottom, chartHeight);
    int closeY = priceToY(klines[i].close, minVal, range, chartBottom, chartHeight);
    int highY = priceToY(klines[i].high, minVal, range, chartBottom, chartHeight);
    int lowY = priceToY(klines[i].low, minVal, range, chartBottom, chartHeight);

    uint16_t candleColor;
    if (results[i].zone == 1) {
      candleColor = (results[i].signal == 1) ? TFT_BLUE : TFT_GREEN;
    } else {
      candleColor = (results[i].signal == -1) ? TFT_YELLOW : TFT_RED;
    }

    tft.drawLine(x + (barWidth / 2), highY, x + (barWidth / 2), lowY, candleColor);

    int topY = (openY < closeY) ? openY : closeY;
    int bottomY = (openY > closeY) ? openY : closeY;
    int bodyH = bottomY - topY;
    if (bodyH < 1) bodyH = 1;
    tft.fillRect(x, topY, barWidth, bodyH, candleColor);

    if (i > 0) {
      int prevX = startX + ((i - 1) * (barWidth + 1)) + (barWidth / 2);
      int currentX = x + (barWidth / 2);
      int prevFastY = priceToY(results[i - 1].emaFast, minVal, range, chartBottom, chartHeight);
      int prevSlowY = priceToY(results[i - 1].emaSlow, minVal, range, chartBottom, chartHeight);
      int fastY = priceToY(results[i].emaFast, minVal, range, chartBottom, chartHeight);
      int slowY = priceToY(results[i].emaSlow, minVal, range, chartBottom, chartHeight);

      tft.drawLine(prevX, prevFastY, currentX, fastY, tft.color565(0, 255, 255));
      tft.drawLine(prevX, prevSlowY, currentX, slowY, tft.color565(255, 105, 180));
    }
  }

  tft.setTextColor(TFT_DARKGREY, TFT_BLACK);
  tft.setTextSize(1);
  tft.setCursor(startX, chartTop - 8);
  tft.print("Max: ");
  tft.print(maxVal, 2);
  tft.setCursor(startX, chartBottom - 5);
  tft.print("Min: ");
  tft.print(minVal, 2);
}

static void drawCDCError(const String &message, int httpCode) {
  tft.fillScreen(TFT_BLACK);
  tft.setTextDatum(MC_DATUM);
  tft.setTextColor(tft.color565(247, 147, 26), TFT_BLACK);
  tft.setTextSize(2);
  tft.drawString("BTC CDC", tft.width() / 2, 48);

  tft.setTextColor(TFT_RED, TFT_BLACK);
  tft.setTextSize(1);
  tft.drawString("Failed to load Klines", tft.width() / 2, 92);
  tft.drawString(message, tft.width() / 2, 112);

  if (httpCode != 0) {
    String codeText = String("HTTP Code: ") + String(httpCode);
    tft.drawString(codeText, tft.width() / 2, 132);
  }

  tft.setTextDatum(TL_DATUM);
}

void showPage26() {
  tft.fillScreen(TFT_BLACK);
  tft.setTextDatum(MC_DATUM);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextSize(1);
  String loadingText = String("Loading BTC CDC ") + currentCDCPageIntervalLabel() + "...";
  tft.drawString(loadingText, tft.width() / 2, 112);
  tft.setTextDatum(TL_DATUM);

  CDCKlineData klines[CDC_PAGE_KLINE_COUNT];
  CDCPageResult results[CDC_PAGE_KLINE_COUNT];
  int count = 0;
  int httpCode = 0;
  String error = "";

  fetchBTCKlineData(klines, count, httpCode, error);

  if (count <= 0) {
    drawCDCError(error, httpCode);
    return;
  }

  calculateCDC(klines, count, results);
  int lastIndex = count - 1;

  drawCDCHeader(results[lastIndex].zone, klines[lastIndex].close);
  drawCDCChart(klines, results, count);
}

bool handlePage26Touch(int x, int y) {
  if (!isCDCIntervalButtonHit(x, y)) {
    return false;
  }

  cdcPageIntervalIndex = (cdcPageIntervalIndex + 1) % CDC_PAGE_INTERVAL_COUNT;
  Serial.print("CDC timeframe: ");
  Serial.println(currentCDCPageIntervalLabel());
  showPage26();
  lastUpdateTime = millis();
  pageChangeTime = millis();
  return true;
}
