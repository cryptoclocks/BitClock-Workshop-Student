#include "Pages.h"
#include "AppConfig.h"
#include "JpegHelper.h"
#include <SD.h>
#include <time.h>

static void centerText(TFT_eSPI& screen, const String& text, int y, int font, uint16_t color) {
  screen.setTextDatum(MC_DATUM);
  screen.setTextColor(color, TFT_BLACK);
  screen.drawString(text, screen.width() / 2, y, font);
  screen.setTextDatum(TL_DATUM);
}

static String formatPrice(float value) {
  if (value >= 1000000) return String(value / 1000000.0f, 2) + "M";
  if (value >= 1000) return String(value, 0);
  if (value >= 10) return String(value, 2);
  return String(value, 4);
}

void showWifiSetupPage(TFT_eSPI& screen, const String& apName) {
  screen.fillScreen(TFT_BLACK);
  centerText(screen, "CONNECT WI-FI", 45, 4, TFT_CYAN);
  centerText(screen, "1. Open Wi-Fi on your phone", 95, 2, TFT_WHITE);
  centerText(screen, "2. Connect to", 125, 2, TFT_WHITE);
  centerText(screen, apName, 155, 4, TFT_YELLOW);
  centerText(screen, "3. Select your 2.4 GHz Wi-Fi", 202, 2, TFT_WHITE);
}

void showWifiConnectedPage(TFT_eSPI& screen, const String& ipAddress) {
  screen.fillScreen(TFT_BLACK);
  centerText(screen, "WI-FI CONNECTED", 80, 4, TFT_GREEN);
  centerText(screen, "IP: " + ipAddress, 125, 2, TFT_WHITE);
  centerText(screen, "Loading Bitkub data...", 175, 2, TFT_CYAN);
}

void showProfilePage(TFT_eSPI& screen, bool sdReady) {
  uint16_t background = screen.color565(13, 31, 52);
  screen.fillScreen(background);
  bool imageDrawn = sdReady && drawSdJpeg(screen, PROFILE_IMAGE, 12, 18);
  if (!imageDrawn) {
    screen.fillCircle(60, 66, 44, screen.color565(41, 74, 107));
    screen.setTextDatum(MC_DATUM);
    screen.setTextColor(TFT_CYAN, screen.color565(41, 74, 107));
    screen.drawString("CC", 60, 66, 4);
  }

  screen.setTextDatum(TL_DATUM);
  screen.setTextColor(TFT_WHITE, background);
  screen.drawString(PROFILE_MOTTO, 120, 22, 2);
  screen.setTextColor(TFT_CYAN, background);
  screen.drawString(PROFILE_NAME, 120, 58, 4);
  screen.setTextColor(TFT_WHITE, background);
  screen.drawString(PROFILE_ROLE, 120, 98, 2);
  screen.drawFastHLine(16, 138, 288, screen.color565(55, 87, 118));
  screen.setTextColor(screen.color565(188, 205, 225), background);
  screen.drawString(PROFILE_COMPANY, 16, 162, 4);

  struct tm timeInfo;
  if (getLocalTime(&timeInfo, 50)) {
    char timeText[24];
    strftime(timeText, sizeof(timeText), "%H:%M  %d/%m/%Y", &timeInfo);
    screen.drawString(timeText, 16, 210, 2);
  }
}

void showCoinPage(TFT_eSPI& screen, const Coin& coin, size_t coinIndex) {
  screen.fillScreen(TFT_BLACK);
  screen.drawRoundRect(10, 8, screen.width() - 20, screen.height() - 16, 13, coin.color);
  screen.setTextDatum(TL_DATUM);
  screen.setTextColor(coin.color, TFT_BLACK);
  screen.drawString(coin.symbol, 22, 20, 4);
  screen.setTextColor(TFT_WHITE, TFT_BLACK);
  screen.drawString("THB", 250, 24, 2);
  centerText(screen, coin.name, 65, 2, TFT_LIGHTGREY);

  if (!coin.hasData) {
    centerText(screen, "Waiting for Bitkub API", 128, 2, TFT_YELLOW);
    centerText(screen, "Check Wi-Fi connection", 160, 2, TFT_LIGHTGREY);
  } else {
    centerText(screen, formatPrice(coin.lastPrice) + " THB", 120, 6, TFT_WHITE);
    uint16_t changeColor = coin.changePercent >= 0 ? TFT_GREEN : TFT_RED;
    String change = String(coin.changePercent >= 0 ? "+" : "") + String(coin.changePercent, 2) + "%";
    centerText(screen, change, 161, 4, changeColor);
    screen.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
    screen.drawString("LOW  " + formatPrice(coin.low24h), 23, 199, 2);
    screen.drawRightString("HIGH  " + formatPrice(coin.high24h), 297, 199, 2);
  }

  centerText(screen, "BITKUB  " + String(coinIndex + 1) + "/" + String(COIN_COUNT), 226, 1, TFT_DARKGREY);
}

static int priceToY(float value, float minValue, float range, int top, int height) {
  int y = top + height - ((value - minValue) / range) * height;
  return constrain(y, top, top + height);
}

void showCdcPage(TFT_eSPI& screen, const CdcData& data) {
  screen.fillScreen(TFT_BLACK);
  screen.setTextDatum(TL_DATUM);
  screen.setTextColor(TFT_WHITE, TFT_BLACK);
  screen.drawString("CDC Action Zone", 10, 8, 4);
  screen.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
  screen.drawString("BTC/THB  4H  EMA 12/26", 11, 39, 2);

  if (!data.hasData || data.count == 0) {
    centerText(screen, "CDC data unavailable", 112, 2, TFT_RED);
    centerText(screen, data.error, 142, 2, TFT_LIGHTGREY);
    return;
  }

  uint16_t zoneColor = data.buyZone ? TFT_GREEN : TFT_RED;
  screen.fillRoundRect(244, 7, 66, 45, 9, zoneColor);
  screen.setTextDatum(MC_DATUM);
  screen.setTextColor(data.buyZone ? TFT_BLACK : TFT_WHITE, zoneColor);
  screen.drawString(data.buyZone ? "BUY" : "SELL", 277, 30, 4);

  const int chartX = 10;
  const int chartY = 68;
  const int chartWidth = 300;
  const int chartHeight = 158;
  float minValue = data.candles[0].low;
  float maxValue = data.candles[0].high;
  for (size_t i = 1; i < data.count; i++) {
    minValue = min(minValue, data.candles[i].low);
    maxValue = max(maxValue, data.candles[i].high);
  }
  float range = maxValue - minValue;
  if (range <= 0) range = 1;
  minValue -= range * 0.05f;
  maxValue += range * 0.05f;
  range = maxValue - minValue;

  float xStep = (float)chartWidth / data.count;
  int bodyWidth = max(2, (int)xStep - 1);
  for (size_t i = 0; i < data.count; i++) {
    int x = chartX + i * xStep;
    int openY = priceToY(data.candles[i].open, minValue, range, chartY, chartHeight);
    int closeY = priceToY(data.candles[i].close, minValue, range, chartY, chartHeight);
    int highY = priceToY(data.candles[i].high, minValue, range, chartY, chartHeight);
    int lowY = priceToY(data.candles[i].low, minValue, range, chartY, chartHeight);
    bool candleBuyZone = data.emaFast[i] > data.emaSlow[i];
    uint16_t candleColor = candleBuyZone ? TFT_GREEN : TFT_RED;
    screen.drawFastVLine(x + bodyWidth / 2, highY, max(1, lowY - highY), candleColor);
    int bodyTop = min(openY, closeY);
    screen.fillRect(x, bodyTop, bodyWidth, max(1, abs(closeY - openY)), candleColor);

    if (i > 0) {
      int previousX = chartX + (i - 1) * xStep + bodyWidth / 2;
      int currentX = x + bodyWidth / 2;
      int previousFastY = priceToY(data.emaFast[i - 1], minValue, range, chartY, chartHeight);
      int currentFastY = priceToY(data.emaFast[i], minValue, range, chartY, chartHeight);
      int previousSlowY = priceToY(data.emaSlow[i - 1], minValue, range, chartY, chartHeight);
      int currentSlowY = priceToY(data.emaSlow[i], minValue, range, chartY, chartHeight);
      screen.drawLine(previousX, previousFastY, currentX, currentFastY, TFT_CYAN);
      screen.drawLine(previousX, previousSlowY, currentX, currentSlowY, TFT_MAGENTA);
    }
  }
  screen.setTextDatum(TL_DATUM);
}

bool showSdSlidePage(TFT_eSPI& screen, uint8_t slideNumber, bool sdReady) {
  screen.fillScreen(TFT_BLACK);
  if (!sdReady) {
    showSdMissingPage(screen);
    return false;
  }

  String path = "/slides/slide" + String(slideNumber) + ".jpg";
  return drawSdJpeg(screen, path.c_str(), 0, 0);
}

void showSdMissingPage(TFT_eSPI& screen) {
  screen.fillScreen(TFT_BLACK);
  centerText(screen, "SD SLIDES", 72, 4, TFT_CYAN);
  centerText(screen, "Add 320x240 JPEG files", 120, 2, TFT_WHITE);
  centerText(screen, "/slides/slide1.jpg", 153, 2, TFT_YELLOW);
  centerText(screen, "up to slide8.jpg", 181, 2, TFT_LIGHTGREY);
}
