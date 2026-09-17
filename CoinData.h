#pragma once
#include <Arduino.h>

const size_t COIN_COUNT = 4;

struct Coin {
  const char* symbol;       // ชื่อย่อที่แสดงบนจอ เช่น BTC
  const char* apiSymbol;    // คู่เหรียญของ Bitkub เช่น BTC_THB
  const char* name;
  float lastPrice;
  float high24h;
  float low24h;
  float changePercent;
  uint16_t color;
  bool hasData;
};

extern Coin coins[COIN_COUNT];

// เหรียญที่หน้า Local Settings เลือกได้ (ทั้งหมดเป็นคู่ THB ของ Bitkub)
const char* supportedCoinSymbol(size_t index);
const char* supportedCoinName(size_t index);
size_t supportedCoinCount();
bool setCoinAt(size_t slot, const String& symbol);
