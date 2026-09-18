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
  if (value >= 100000) return String(value, 0);
  if (value >= 10000) return String(value, 1);
  if (value >= 1000) return String(value, 2);
  if (value >= 10) return String(value, 2);
  return String(value, 4);
}

static String formatWithComma(String value) {
  int decimal = value.indexOf('.');
  int integerEnd = decimal < 0 ? value.length() : decimal;
  String formatted;
  int digits = 0;
  for (int i = integerEnd - 1; i >= 0; i--) {
    if (digits > 0 && digits % 3 == 0) formatted = "," + formatted;
    formatted = value[i] + formatted;
    digits++;
  }
  if (decimal >= 0) formatted += value.substring(decimal);
  return formatted;
}

static void maskProfileToCircle(TFT_eSPI& screen) {
  const int centerX = 60;
  const int centerY = 60;
  const int radius = 50;
  for (int x = centerX - radius; x <= centerX + radius; x++) {
    for (int y = centerY - radius; y <= centerY + radius; y++) {
      int dx = x - centerX;
      int dy = y - centerY;
      if (dx * dx + dy * dy > radius * radius - 100) screen.drawPixel(x, y, TFT_BLACK);
    }
  }
}

static void drawCoinMark(TFT_eSPI& screen, const Coin& coin, int centerX, int centerY) {
  screen.fillCircle(centerX, centerY, 24, coin.color);
  screen.drawCircle(centerX, centerY, 24, TFT_WHITE);
  screen.setTextDatum(MC_DATUM);
  screen.setTextColor(TFT_BLACK, coin.color);
  screen.drawString(String(coin.symbol).substring(0, 1), centerX, centerY, 4);
  screen.setTextDatum(TL_DATUM);
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
  screen.fillScreen(TFT_BLACK);
  bool imageDrawn = sdReady && drawSdJpeg(screen, PROFILE_IMAGE, 10, 10);
  if (!imageDrawn) {
    screen.fillCircle(60, 60, 50, screen.color565(247, 147, 26));
    screen.setTextDatum(MC_DATUM);
    screen.setTextColor(TFT_BLACK, screen.color565(247, 147, 26));
    screen.drawString("CC", 60, 60, 4);
  } else {
    maskProfileToCircle(screen);
  }

  screen.setTextDatum(TL_DATUM);
  size_t mottoLength = strlen(PROFILE_MOTTO);
  int mottoFont = mottoLength <= 13 ? 3 : 2;
  int mottoWidth = screen.textWidth(PROFILE_MOTTO, mottoFont);
  screen.setTextColor(screen.color565(255, 174, 0), TFT_BLACK);
  screen.drawString(PROFILE_MOTTO, 107 + (208 - mottoWidth) / 2, 15, mottoFont);
  screen.setTextColor(screen.color565(255, 174, 0), TFT_BLACK);
  screen.drawString(PROFILE_NAME, 10, 140, 2);
  screen.setTextColor(TFT_WHITE, TFT_BLACK);
  screen.drawString(PROFILE_ROLE, 10, 175, 2);
  screen.drawString(PROFILE_COMPANY, 10, 205, 2);

  struct tm timeInfo;
  if (getLocalTime(&timeInfo, 50)) {
    char timeText[10];
    char dateText[16];
    strftime(timeText, sizeof(timeText), "%H:%M", &timeInfo);
    strftime(dateText, sizeof(dateText), "%d/%m/%Y", &timeInfo);
    screen.setTextColor(TFT_WHITE, TFT_BLACK);
    screen.drawString(timeText, 152, 60, 5);
    screen.drawString(dateText, 165, 110, 2);
  }
}

void showCoinPage(TFT_eSPI& screen, const Coin& coin, size_t coinIndex) {
  screen.fillScreen(TFT_BLACK);
  const int boxWidth = 95;
  const int boxHeight = 48;
  screen.drawRoundRect(20, 14, boxWidth, boxHeight, 6, coin.color);
  screen.drawRoundRect(205, 14, boxWidth, boxHeight, 6, coin.color);
  screen.setTextDatum(TL_DATUM);
  screen.setTextColor(TFT_WHITE, TFT_BLACK);
  int coinTextX = 67 - screen.textWidth(coin.symbol, 2) / 2;
  screen.drawString(coin.symbol, coinTextX, 30, 2);
  screen.setTextColor(TFT_WHITE, TFT_BLACK);
  screen.drawString("THB", 252 - screen.textWidth("THB", 2) / 2, 30, 2);
  drawCoinMark(screen, coin, 160, 34);

  if (!coin.hasData) {
    centerText(screen, "Waiting for Bitkub API", 125, 2, TFT_YELLOW);
    centerText(screen, "Check Wi-Fi connection", 153, 2, TFT_LIGHTGREY);
  } else {
    String price = formatWithComma(formatPrice(coin.lastPrice));
    int priceFont = price.length() > 9 ? 4 : 5;
    centerText(screen, price, price.length() > 9 ? 125 : 115, priceFont, coin.color);
    uint16_t changeColor = coin.changePercent >= 0 ? TFT_GREEN : TFT_RED;
    String change = String(coin.changePercent >= 0 ? "+" : "") + String(coin.changePercent, 2) + "%";
    screen.setTextColor(changeColor, TFT_BLACK);
    screen.drawString(change, 310 - screen.textWidth(change, 1), 85, 1);
    screen.setTextColor(TFT_GREEN, TFT_BLACK);
    screen.drawString("Lowest", 30, 180, 2);
    screen.setTextColor(TFT_RED, TFT_BLACK);
    screen.drawString("Highest", 223, 180, 2);
    screen.setTextColor(TFT_WHITE, TFT_BLACK);
    String low = formatWithComma(formatPrice(coin.low24h));
    String high = formatWithComma(formatPrice(coin.high24h));
    screen.drawString(low, 60 - screen.textWidth(low, 2) / 2, 200, 2);
    screen.drawString(high, 255 - screen.textWidth(high, 2) / 2, 200, 2);
  }
  centerText(screen, "BITKUB  " + String(coinIndex + 1) + "/" + String(COIN_COUNT), 232, 1, TFT_DARKGREY);
}

static int priceToY(float value, float minValue, float range, int top, int height) {
  int y = top + height - ((value - minValue) / range) * height;
  return constrain(y, top, top + height);
}

void showCdcPage(TFT_eSPI& screen, const CdcData& data) {
  screen.fillScreen(TFT_BLACK);
  screen.setTextDatum(TL_DATUM);
  screen.setTextColor(TFT_WHITE, TFT_BLACK);
  screen.drawString("CDC ActionZone 3.0", 8, 7, 2);

  if (!data.hasData || data.count == 0) {
    centerText(screen, "CDC data unavailable", 112, 2, TFT_RED);
    centerText(screen, data.error, 142, 2, TFT_LIGHTGREY);
    return;
  }

  uint16_t zoneColor = data.buyZone ? TFT_GREEN : TFT_RED;
  screen.fillRoundRect(238, 0, 82, 82, 8, zoneColor);
  screen.setTextDatum(MC_DATUM);
  screen.setTextColor(data.buyZone ? TFT_BLACK : TFT_WHITE, zoneColor);
  screen.drawString(data.buyZone ? "BUY" : "SELL", 279, 41, 4);
  screen.setTextDatum(TL_DATUM);
  screen.drawRoundRect(259, 81, 40, 18, 4, TFT_WHITE);
  screen.setTextColor(TFT_WHITE, TFT_BLACK);
  screen.drawString("4H", 272, 86, 1);
  Coin btc = {"BTC", "BTC_THB", "Bitcoin", 0, 0, 0, 0, screen.color565(247, 147, 26), false};
  drawCoinMark(screen, btc, 34, 58);
  String latestPrice = formatWithComma(formatPrice(data.candles[data.count - 1].close));
  int priceFont = latestPrice.length() > 9 ? 2 : 3;
  screen.setTextColor(screen.color565(247, 147, 26), TFT_BLACK);
  screen.drawString(latestPrice, 68, priceFont == 3 ? 47 : 51, priceFont);

  const int chartX = 10;
  const int chartY = 106;
  const int chartWidth = 300;
  const int chartHeight = 126;
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
  screen.setTextColor(TFT_DARKGREY, TFT_BLACK);
  screen.drawString("Max: " + String(maxValue, 2), chartX, chartY - 8, 1);
  screen.drawString("Min: " + String(minValue, 2), chartX, chartY + chartHeight - 5, 1);
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
