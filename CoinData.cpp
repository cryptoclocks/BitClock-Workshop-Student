#include "CoinData.h"

// ราคาจะถูกเติมจาก Bitkub Public API หลังต่อ Wi-Fi สำเร็จ
Coin coins[COIN_COUNT] = {
  {"BTC",  "BTC_THB",  "Bitcoin",   0, 0, 0, 0, 0xFBE0, false},
  {"ETH",  "ETH_THB",  "Ethereum",  0, 0, 0, 0, 0x07FF, false},
  {"KUB",  "KUB_THB",  "Bitkub Coin",0, 0, 0, 0, 0x07E0, false},
  {"USDT", "USDT_THB", "Tether",    0, 0, 0, 0, 0xC618, false}
};

struct SupportedCoin {
  const char* symbol;
  const char* apiSymbol;
  const char* name;
  uint16_t color;
};

static const SupportedCoin SUPPORTED_COINS[] = {
  {"BTC",  "BTC_THB",  "Bitcoin",     0xFBE0},
  {"ETH",  "ETH_THB",  "Ethereum",    0x07FF},
  {"KUB",  "KUB_THB",  "Bitkub Coin", 0x07E0},
  {"USDT", "USDT_THB", "Tether",      0xC618},
  {"SOL",  "SOL_THB",  "Solana",      0xB81F},
  {"XRP",  "XRP_THB",  "XRP",         0xFFFF},
  {"ADA",  "ADA_THB",  "Cardano",     0x05BF},
  {"DOGE", "DOGE_THB", "Dogecoin",    0xFEA0}
};

size_t supportedCoinCount() {
  return sizeof(SUPPORTED_COINS) / sizeof(SUPPORTED_COINS[0]);
}

const char* supportedCoinSymbol(size_t index) {
  return index < supportedCoinCount() ? SUPPORTED_COINS[index].symbol : "BTC";
}

const char* supportedCoinName(size_t index) {
  return index < supportedCoinCount() ? SUPPORTED_COINS[index].name : "Bitcoin";
}

bool setCoinAt(size_t slot, const String& symbol) {
  if (slot >= COIN_COUNT) return false;
  for (size_t i = 0; i < supportedCoinCount(); i++) {
    if (symbol.equalsIgnoreCase(SUPPORTED_COINS[i].symbol)) {
      coins[slot] = {SUPPORTED_COINS[i].symbol, SUPPORTED_COINS[i].apiSymbol,
                     SUPPORTED_COINS[i].name, 0, 0, 0, 0,
                     SUPPORTED_COINS[i].color, false};
      return true;
    }
  }
  return false;
}
