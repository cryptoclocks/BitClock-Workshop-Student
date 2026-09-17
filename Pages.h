#pragma once
#include <Arduino.h>
#include <TFT_eSPI.h>
#include "CoinData.h"
#include "BitkubApi.h"

void showWifiSetupPage(TFT_eSPI& screen, const String& apName);
void showWifiConnectedPage(TFT_eSPI& screen, const String& ipAddress);
void showProfilePage(TFT_eSPI& screen, bool sdReady);
void showCoinPage(TFT_eSPI& screen, const Coin& coin, size_t coinIndex);
void showCdcPage(TFT_eSPI& screen, const CdcData& data);
bool showSdSlidePage(TFT_eSPI& screen, uint8_t slideNumber, bool sdReady);
void showSdMissingPage(TFT_eSPI& screen);
