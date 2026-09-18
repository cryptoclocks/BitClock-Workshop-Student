#ifndef GLOBAL_PARAMETER_H
#define GLOBAL_PARAMETER_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include <FS.h>
#include "GlobalModule.h"
#include "cc_aes.h"
#include <SD.h>
#include <HTTPClient.h>
#include <time.h>
#include <WiFiClientSecure.h>
#include "cert.h"
#include <AnimatedGIF.h>
#include "doneg96.h"  // ต้อง include ตัวนี้ให้แน่ใจว่ามี buffer
#define GIF_IMAGE doneg96
extern AnimatedGIF gif;        // ตัวแปร global gif


// 🪙 CRYPTO COIN SETTINGS

extern int coinType;

// 📊 USAGE / ALERT / ADS
extern int usage_total;
extern bool ads_enabled;
extern String current_ads;

// 💰 OTHER ASSET PARAMETERS
extern int bar_sell, bar_buy;
extern int jewelry_sell, jewelry_buy;
extern int last_bar_sell, last_bar_buy;
extern int last_jewelry_sell, last_jewelry_buy;
extern bool waitforprice, waitforclick, pageUpdate;

// 🌐 SERVER CONFIGURATION
extern char menuId;

// 📈 PRICE / MARKET DATA
// ราคาพื้นฐาน (base currency เสมอ เช่น USD หรือ THB ตามชนิดเหรียญ)
extern float price[4];
extern float highPrice[4];
extern float lowPrice[4];
extern float priceChange[4];
extern float priceChangePercent[4];

// อัตราแลกเปลี่ยน: [เหรียญ][สกุลเงิน] เช่น exRate[0][1] = USD → THB สำหรับเหรียญที่ 0
extern float exRate[4][2];
extern float USDTHBRate;

// ราคาที่ใช้แสดงผลตาม currencyIndex
extern float displayPrice[4];
extern float displayHighPrice[4];
extern float displayLowPrice[4];
extern float displayChange[4];
extern float displayChangePercent[4];


extern String currency[2], coin[4], type[4], color[4], market[4];
extern float coinHigh[4][8], coinLow[4][8];
extern bool coinLowStatus[4][8], coinHighStatus[4][8];
extern int alertCountHigh[4];
extern int alertCountLow[4];
extern bool isTaskOn;
extern unsigned long fetchDelay;
extern String api;

// ⏲️ TIME / SCHEDULING
extern unsigned long lastNtpUpdate, ntpUpdateInterval, lastUpdateTime;
extern long intervals[5];
extern int currentIntervalIndex;
extern long timeouts[5];
extern int currentTimeoutIndex;

// 🧩 SHARED OBJECTS
extern StaticJsonDocument<8192> jsonData;
extern File file;

// ⚙️ CONFIG.JSON
extern String firmwareVersion, deviceId, plan, deviceType, deviceModel;
extern int brightnessLevel, updateInterval, timeout;
extern String pageMode;
extern int pageIndex, coinIndex, currencyIndex;
extern bool lotteryAlert, duplottery, lineAlert, soundAlert;

// 🌐 WIFI.JSON
extern String wifi_ssid, wifi_password;

// 👤 PROFILE SETTINGS
extern String profile_firstname, profile_lastname, profile_nickname;
extern String profile_position, profile_company, profile_motto;
extern String profile_bgcolor, profile_namecolor, profile_detailcolor;
extern String profile_mottocolor, profile_datetimecolor;
extern String profile_refer;
extern StaticJsonDocument<512> profile_referring;


// 📱 CONTACT
extern String contact_line;
extern String contact_whatsapp;
extern String contact_tel;
extern String contact_email;

// 🌐 SOCIAL
extern String social_facebook;
extern String social_instagram;
extern String social_youtube;
extern String social_tiktok;


// 📄 PAGE SETTINGS

extern bool profileToggle, coinToggle, screensaverToggle, cdcToggle;
extern unsigned long pageChangeTime, pageDurations[4], durationTask;
extern unsigned int durationTaskCount;
extern String profileDuration;
extern String coinDuration;
extern String screensaverDuration;
extern String cdcDuration;
extern bool serverOn;

// 🚨 ALERT PAGE SETTINGS
#define MAX_COINS 4
#define MAX_ALERTS 8
extern String currencyAlert, inputText, alertCoin;
extern int currentAlertPage, alertSelectIndex, inputCursorPos;
extern bool inputMode;
extern unsigned long lastAlertInterval, alertTime;
extern String alertMessage, alertStatus, alertPrice;
extern bool alertEnable;
extern bool pauseAlert;
extern int alertI, alertJ;

// 🪙 TOKEN / EXCHANGE

extern bool editingInput;
extern float coinInput, coinOutput;
extern bool coinReceive;
extern byte currentWallet;
extern String lastMemoSat;
extern String lastMemoUsdt;
extern String lastMemoCct;
extern bool mySuccess;

// 📊 RATE CONVERSION
extern const int numChannels;
extern float rates[];
extern char currentRate;

// 🖼️ DISPLAY / GRAPHICS
extern File bmpFile;
extern int16_t bmpWidth, bmpHeight;
extern uint16_t* lineBuffer;
extern int yOffset1;

// 🔧 API PATH / CONFIG KEYS
extern String configKeys[2], api_url[2], price_path[2];
extern String high_path[2], low_path[2], change_path[2], percent_path[2];

// 📱 QR / PAYMENT
extern bool isStatic, accType;
extern char userPromptpay[];
extern float amounts;
extern int maxCoin;
extern String currentCurrencyKey;
extern String currentCurrency ; 
extern int selectedPrice;
extern float payPrice;
// 🛠️ OTHER VARIABLES
extern bool isLowPowerMode;
extern bool sdStatus;
extern int keyWidth, keyHeight, keyStartX, keyStartY, keyPadding;
extern String macAddress ; 
extern bool rotate180;
extern unsigned long updateClickInterval;
extern String ipAddress;
extern HTTPClient http3;
extern String currentAds;
extern float lastPrice;

// SETTINGS (ใน object settings)
extern bool settings_alert;
extern bool settings_game;
extern bool settings_horo;
extern String settings_plan;


extern String region;
extern String timeZone;
// ตัวแปรค่าเงินที่ใช้ในระบบ
extern String currency1;
extern String currency2;


extern int txCount;

//topup
extern float usdRate ;
extern float btcPrice ;
extern float stripePrice, usdtPrice, cctPrice;
extern int satoshiPrice;
extern float exPayRate ;

extern String encodedClientId ;
// ตัวแปร global (ประกาศ extern)
extern String lastReserveId;
extern String lastAddReserveId;
extern int ccp_balance;
extern bool reserveInProgress;
extern bool addInProgress;


// สถานะรอการชำระ
extern bool stripePending;
extern bool blinkPending;
extern bool usdtPending;
extern bool cctPending;

// reference ล่าสุด
extern String stripeReference;
extern String blinkReference;
extern String usdtReference;
extern String cctReference;

// จำนวนเงินที่รอชำระ (THB หรืออื่น ๆ ตาม context)
extern float stripeAmount;
extern float satAmount;
extern float usdtAmount;
extern float cctAmount;
extern int pointAmount;

extern String lastUsdtTx ;
extern String lastSatTx ;
extern String lastCctTx ;

extern String baseFolder;
extern String githubBase;
extern bool inversion;

extern bool paymentConfirm ;
extern bool paymentProcess ;
extern unsigned long lastPaymentRequestTime;
extern const unsigned long paymentTimeout; // 5 วินาที
extern int paymentAttemptCount;
extern const int maxPaymentAttempts;

extern const uint8_t BUZZER;


#endif // GLOBAL_PARAMETER_H
