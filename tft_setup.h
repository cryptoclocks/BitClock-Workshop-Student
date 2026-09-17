#pragma once

// TFT_eSPI configuration สำหรับ ESP32-2432S028R (2.8 นิ้ว / ILI9341)
// TFT_eSPI จะค้นหาไฟล์ชื่อ tft_setup.h ในโฟลเดอร์ Sketch อัตโนมัติ
#define USER_SETUP_ID 2432028
#define ILI9341_2_DRIVER

#define TFT_WIDTH  240
#define TFT_HEIGHT 320

#define TFT_MISO 12
#define TFT_MOSI 13
#define TFT_SCLK 14
#define TFT_CS   15
#define TFT_DC    2
#define TFT_RST  -1

#define LOAD_GLCD
#define LOAD_FONT2
#define LOAD_FONT4
#define LOAD_FONT6
#define LOAD_FONT7
#define LOAD_FONT8
#define LOAD_GFXFF

#define SMOOTH_FONT
#define SPI_FREQUENCY       55000000
#define SPI_READ_FREQUENCY  20000000
#define SUPPORT_TRANSACTIONS
