#pragma once
#include <Arduino.h>

// ----- ESP32-2432S028R Hardware -----
const uint8_t TFT_BACKLIGHT_PIN = 21;
const uint8_t SD_CS_PIN = 5;
const uint8_t SD_SCLK_PIN = 18;
const uint8_t SD_MISO_PIN = 19;
const uint8_t SD_MOSI_PIN = 23;

// ----- ระยะเวลาแต่ละหน้า -----
const unsigned long PROFILE_PAGE_MS = 10000;
const unsigned long COIN_PAGE_MS = 8000;       // ต่อเหรียญ
const unsigned long CDC_PAGE_MS = 15000;
const unsigned long SD_SLIDE_MS = 8000;        // ต่อภาพ
const unsigned long PRICE_REFRESH_MS = 60000;
const unsigned long CDC_REFRESH_MS = 5UL * 60UL * 1000UL;

// ----- Profile -----
const char PROFILE_NAME[] = "Crypto Clock";
const char PROFILE_ROLE[] = "ESP32 Learning Edition";
const char PROFILE_COMPANY[] = "BitClock Workshop";
const char PROFILE_MOTTO[] = "Learn by building";
const char PROFILE_IMAGE[] = "/profile.jpg";   // JPEG 96x96 บน SD Card

// ----- SD slideshow -----
const uint8_t MAX_SD_SLIDES = 8;
// ตั้งชื่อไฟล์ /slides/slide1.jpg ถึง /slides/slide8.jpg

// ----- CDC Action Zone -----
const char CDC_SYMBOL[] = "BTC_THB";
const char CDC_RESOLUTION[] = "240";           // 4 ชั่วโมง
const uint8_t CDC_CANDLE_COUNT = 60;
const uint8_t CDC_EMA_FAST = 12;
const uint8_t CDC_EMA_SLOW = 26;
