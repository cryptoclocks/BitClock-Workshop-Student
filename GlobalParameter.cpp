#include <Arduino.h>
#include <ArduinoJson.h>
#include <FS.h>
#include "GlobalModule.h"
#include "GlobalParameter.h"
#include <SD.h>
#include <HTTPClient.h>

// รวมตัวแปรจากไฟล์เดิมทั้งหมด พร้อมจัดหมวดหมู่อย่างชัดเจน

// ┏━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┓
// ┃ 🪙 CRYPTO COIN SETTINGS (7)        ┃
// ┗━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┛


int coinType = 0;

// ┏━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┓
// ┃ 📊 USAGE / ALERT / ADS (4)          ┃
// ┗━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┛
int usage_total = 0;

bool ads_enabled = true;
String current_ads = "0";

// ┏━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┓
// ┃ 💰 OTHER ASSET PARAMETERS (11)     ┃
// ┗━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┛
int bar_sell = 0, bar_buy = 0;
int jewelry_sell = 0, jewelry_buy = 0;
int last_bar_sell = 0, last_bar_buy = 0;
int last_jewelry_sell = 0, last_jewelry_buy = 0;
bool waitforprice = false, waitforclick = false;
bool pageUpdate = false;

// ┏━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┓
// ┃ 🌐 SERVER CONFIGURATION (2)        ┃
// ┗━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┛
char menuId = 0;

// ┏━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┓
// ┃ 📈 PRICE / MARKET DATA (20)        ┃
// ┗━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┛
float price[4] = {0};
float highPrice[4] = {0};
float lowPrice[4] = {0};
float priceChangePercent[4] = {0};
float priceChange[4] = {0};

// --- อัตราแลกเปลี่ยนของแต่ละเหรียญ (อัปเดตแยกกัน) ---
float exRate[4][2] = {
  {1.0, 1.0},  // coin 0
  {1.0, 1.0},  // coin 1
  {1.0, 1.0},  // coin 2
  {1.0, 1.0}   // coin 3
};


// --- ข้อมูลที่แสดงผล (หลังคูณ exRate) ---
float displayPrice[4]              = {0};
float displayHighPrice[4]          = {0};
float displayLowPrice[4]           = {0};
float displayChange[4]             = {0};
float displayChangePercent[4]      = {0};

String currency[2] = {"USDT", "THB"};
String coin[4] = {"BTC", "KUB", "BNB", "DOGE"};
String market[4] = {"NASDAQ", "NASDAQ", "NASDAQ", "NASDAQ"};
String type[4] = {"1", "2", "1", "1"};
String color[4] = {"#FF0000", "#00FF00", "#FF0000", "#00FF00"};
float coinHigh[4][8] = {{0}};
float coinLow[4][8] = {{0}};
int alertCountHigh[4] = {0};
int alertCountLow[4]  = {0};


bool coinLowStatus[4][8] = {{false}};
bool coinHighStatus[4][8] = {{false}};
bool isTaskOn = false;
unsigned long fetchDelay = 0;
String api = "https://api.binance.com/api/v3/ticker/price?symbol=BTCUSDT";

// ┏━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┓
// ┃ ⏲️ TIME / SCHEDULING (7)           ┃
// ┗━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┛
unsigned long lastNtpUpdate = 0, ntpUpdateInterval = 3600000;
unsigned long lastUpdateTime = 0;
long intervals[] = {10, 30, 60, 120, 300};
int currentIntervalIndex = 0;
long timeouts[] = {0, 30, 60, 120, 300};
int currentTimeoutIndex = 0;

// ┏━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┓
// ┃ 🧩 SHARED OBJECTS (2)              ┃
// ┗━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┛
StaticJsonDocument<8192> jsonData;
File file; // Shared File object
// ┏━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┓
// ┃ ⚙️ CONFIG.JSON (21)                 ┃
// ┗━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┛
String firmwareVersion = "4.1.4";
String deviceType = "ccn";
String deviceModel = "2025";
String deviceId = "CCN000000";
String plan = "free";
int brightnessLevel = 100;
int updateInterval = 10000;
int timeout = 30;

String pageMode = "dynamic";
int pageIndex = 0;
int coinIndex = 0;
int currencyIndex = 0;
bool lotteryAlert = false;
bool duplottery = false;
bool lineAlert = false;
bool soundAlert = false;


// ┏━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┓
// ┃ 🌐 WIFI.JSON (2)                    ┃
// ┗━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┛
// Do not ship personal Wi-Fi credentials in the student project.
// The board starts its local setup page until the learner saves their own network.
String wifi_ssid = "";
String wifi_password = "";

// ┏━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┓
// ┃ 👤 PROFILE SETTINGS (13)            ┃
// ┗━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┛
String profile_firstname = "Satoshi";
String profile_lastname = "Nakamoto";
String profile_nickname = "Sat";
String profile_position = "Founder";
String profile_company = "Bitcoin";
String profile_motto = "Don't trust, Verify";
String profile_bgcolor = "#000000";
String profile_namecolor = "#FFAE00";
String profile_detailcolor = "#FFFFFF";
String profile_mottocolor = "#FFAE00";
String profile_datetimecolor = "#FFFFFF";
String profile_refer = "CCN000000";

// 💰 WALLET

// 📱 CONTACT
String contact_line     = "0";
String contact_whatsapp = "0";
String contact_tel      = "0";
String contact_email    = "mailto:mycryptoclock@gmail.com";

// 🌐 SOCIAL
String social_facebook  = "https://facebook.com/mycryptoclock";
String social_instagram = "https://instagram.com/mycryptoclock";
String social_youtube   = "https://youtube.com/@mycryptoclock";
String social_tiktok    = "https://tiktok.com/@mycryptoclock";


StaticJsonDocument<512> profile_referring;

// ┏━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┓
// ┃ 📄 PAGE SETTINGS (11)               ┃
// ┗━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┛

bool profileToggle = true, coinToggle = true, screensaverToggle = false, cdcToggle = false;
String profileDuration = "15";
String coinDuration = "15";
String screensaverDuration = "15";
String cdcDuration = "15";
unsigned long pageChangeTime = 0;
bool serverOn = false;
unsigned long pageDurations[] = {10000, 15000, 20000, 15000};
unsigned long durationTask = 0;
unsigned int durationTaskCount = 0;

// ┏━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┓
// ┃ 🚨 ALERT PAGE SETTINGS (16)         ┃
// ┗━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┛
String currencyAlert = "USDT", inputText = "", alertCoin = "";
int currentAlertPage = 1, alertSelectIndex = 0, inputCursorPos = 0;
bool inputMode = false;
unsigned long lastAlertInterval = 100;
String alertMessage = "", alertStatus = "", alertPrice = "";
unsigned long alertTime;
bool alertEnable = true;
bool pauseAlert= false;
int alertI = 0, alertJ = 0;

// ┏━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┓
// ┃ 🪙 TOKEN / EXCHANGE (10)            ┃
// ┗━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┛

bool editingInput = true;
float coinInput = 0.0, coinOutput = 0.0;
bool coinReceive = false;
byte currentWallet = 0;

bool mySuccess = false;

// ┏━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┓
// ┃ 📊 RATE CONVERSION (4)              ┃
// ┗━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┛
const int numChannels = 7;
float rates[numChannels];
char currentRate = 0;

// ┏━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┓
// ┃ 🖼️ DISPLAY / GRAPHICS (4)           ┃
// ┗━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┛
File bmpFile;
int16_t bmpWidth, bmpHeight;
uint16_t* lineBuffer;
int yOffset1 = 0;



// ┏━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┓
// ┃ 🔧 API PATH / CONFIG KEYS (6)       ┃
// ┗━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┛
String configKeys[2] = {"USDT", "THB"};
String api_url[2] = {
  "https://api.binance.com/api/v3/ticker/24hr?symbol={symbol}USDT",
  "https://api.bitkub.com/api/market/ticker?sym=THB_{symbol}"
};
String price_path[2] = {"lastPrice", "THB_{symbol}.last"};
String high_path[2] = {"highPrice", "THB_{symbol}.high24hr"};
String low_path[2] = {"lowPrice", "THB_{symbol}.low24hr"};
String change_path[2] = {"priceChange", "THB_{symbol}.change"};
String percent_path[2] = {"priceChangePercent", "THB_{symbol}.percentChange"};

// ┏━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┓
// ┃ 📱 QR / PAYMENT (7)                 ┃
// ┗━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┛
// ┗━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┛
bool isStatic = false, accType = false;
char userPromptpay[] = "0992515992";
float amounts = 21.0;
int maxCoin = 2;
String currentCurrencyKey = "currency1";
int selectedPrice = 0;
float payPrice = 0;
// รวมตัวแปรจากไฟล์เดิมทั้งหมด พร้อมจัดหมวดหมู่อย่างชัดเจน

//other
bool isLowPowerMode = false;
bool sdStatus = false;
int keyWidth = 55;
int keyHeight = 55;
int keyStartX = 5;
int keyStartY = 66;
int keyPadding = 5;
String macAddress = "A1:B2:C3:D4:E5:F6";
bool rotate180 = false ;
unsigned long updateClickInterval = 5000;
String ipAddress = "192.168.0.100";
String currentAds = "0";
float lastPrice = 0.0;

//settings plan
bool settings_alert = false;
bool settings_game = false;
bool settings_horo = false;
String settings_plan = "free";

String region = "thailand";          // ค่าเริ่มต้น หากยังไม่โหลดจาก config
String timeZone = "Asia/Bangkok";  // ค่าเริ่มต้น timezone
String currency1 = "USDT";
String currency2 = "THB";
String currentCurrency = currency1 ; 

//topup
float usdRate = 36.0;
float btcPrice = 68000.0;
float stripePrice, usdtPrice, cctPrice;
int satoshiPrice;
float exPayRate = 36.0 ;

int txCount = 0 ;
String encodedClientId = "";

String lastReserveId;
String lastAddReserveId;   // เก็บ reserveId ของ add-point
int ccp_balance = -1;  // ค่าเริ่มต้น -1 เพื่อระบุว่ายังไม่ได้โหลด
bool reserveInProgress = false;
bool addInProgress       = false;

// สถานะการรอชำระ
bool stripePending = false;
bool blinkPending  = false;
bool usdtPending   = false;
bool cctPending    = false;

// reference ล่าสุดของแต่ละช่องทาง
String stripeReference = "";
String blinkReference  = "";
String usdtReference   = "";
String cctReference    = "";

// จำนวนเงินล่าสุดที่ขอชำระ
float stripeAmount = 0.0;
float satAmount  = 0.0;
float usdtAmount   = 0.0;
float cctAmount    = 0.0;// สถานะการรอชำระ
int pointAmount = 0 ;
AnimatedGIF gif;
String lastUsdtTx = "";
String lastSatTx = "";
String lastCctTx = "";


String lastMemoUsdt = "" ; 
String lastMemoCct = "" ; 
String lastMemoSat = "" ; 

String baseFolder = "/";
String githubBase = "/";
bool inversion = false;

bool paymentProcess = false;
bool paymentConfirm = false;
unsigned long lastPaymentRequestTime = 0;
const unsigned long paymentTimeout = 5000; // 5 วินาที
int paymentAttemptCount = 0;
const int maxPaymentAttempts = 3;
float USDTHBRate = 35.0;
const uint8_t BUZZER = 26;
