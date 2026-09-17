#pragma once
#include <Arduino.h>
#include "CoinData.h"
#include "AppConfig.h"

struct Candle {
  float open;
  float high;
  float low;
  float close;
};

struct CdcData {
  Candle candles[CDC_CANDLE_COUNT];
  float emaFast[CDC_CANDLE_COUNT];
  float emaSlow[CDC_CANDLE_COUNT];
  size_t count;
  bool buyZone;
  bool hasData;
  String error;
};

bool fetchBitkubTicker(Coin& coin);
void fetchAllBitkubTickers();
bool fetchBitkubCdcHistory(CdcData& data);
