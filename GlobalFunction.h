#ifndef GLOBAL_FUNCTION_h
#define GLOBAL_FUNCTION_h
#include <XPT2046_Bitbang.h>


void loadBrightnessFromFile();

void startPriceTask() ;
void stopPriceTask() ;
void   gotopage1();
void   gotopage2();
void drawGradientRoundedRect(int x, int y, int w, int h, int radius, uint32_t colorStart, uint32_t colorEnd);
uint32_t fastBlend(uint8_t alpha, uint32_t fgc, uint32_t bgc);
void drawArrow(int startX, int startY, String direction);
void drawQRCode(const char *url , int type , int scale , int ydiff);
void adjustBrightness();
void decreaseBrightness();
void increaseBrightness();
void decreaseInterval() ;
void increaseInterval() ;
void decreaseTimeout();
void increaseTimeout();
void drawKeypad();
void drawCircleWithText(int x, int y, int radius, uint16_t fillColor, uint16_t borderColor, uint16_t txtColor, const char* text);
void fetchAndParseCSV(String url);

void testDisplayPriceOnScreen();
void updateExchangeRate();
void updateDisplayValues();
uint16_t rgbTo565(uint8_t r, uint8_t g, uint8_t b);
void silenceBuzzer();

void beepThreeTimes();
String formatAlertPrice(float priceTemp);

float fetchExchangeRate(String from, String to) ;
#endif
