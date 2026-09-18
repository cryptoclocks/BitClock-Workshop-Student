# CryptoClock Learning Edition

โค้ดใหม่สำหรับสอนภายใน 3 ชั่วโมง อยู่ในโฟลเดอร์นี้เท่านั้น โค้ดต้นฉบับใน `CryptoClockV3` ไม่ถูกแก้ไข

## ฮาร์ดแวร์ที่ใช้

- ESP32-2432S028R พร้อมจอ TFT 2.8 นิ้ว 240x320 และ Resistive Touch
- สาย USB-A ไป USB Type-C ที่รองรับ Data
- เฟรมอะคริลิกสำหรับ ESP32-2432S028R ขนาด 2.8 นิ้ว
- MicroSD Card แบบ FAT32

ไฟล์ `tft_setup.h` ตั้งค่าจอสำหรับบอร์ดรุ่นนี้ไว้ในโฟลเดอร์ Sketch แล้ว:

- TFT: MISO 12, MOSI 13, SCLK 14, CS 15, DC 2, Backlight 21
- MicroSD: SCLK 18, MISO 19, MOSI 23, CS 5

## หน้าจอของระบบ

หน้าหลักวนอัตโนมัติตามลำดับ:

1. Profile
2. ราคาเหรียญแบบตัวเลขตรงกลาง (BTC, ETH, KUB, USDT)
3. CDC Action Zone ของ BTC/THB กรอบเวลา 4 ชั่วโมง
4. ภาพสไลด์จาก SD Card

หน้าต่อ Wi-Fi เป็นหน้าตั้งค่าแยก จะแสดงเมื่อ WiFiManager เปิดเครือข่าย `CryptoClock-Setup`

## Local Web Settings

หลังบอร์ดเชื่อม Wi-Fi แล้ว จะมีหน้า Settings ที่ `http://<IP-ของบอร์ด>/` เช่น `http://192.168.1.42/` ดู IP ได้จาก Serial Monitor บรรทัด `Settings: http://...` หรือหน้าจอ Wi-Fi Connected

หน้า Local Settings ใช้ได้กับโทรศัพท์หรือคอมพิวเตอร์ที่อยู่ Wi-Fi เดียวกับบอร์ด และทำได้ดังนี้:

- เลือกเหรียญที่จะแสดงได้ 4 ช่อง: BTC, ETH, KUB, USDT, SOL, XRP, ADA, DOGE
- เปิด/ปิดหน้า Profile, Coin, CDC และ SD Slide
- ตั้งเวลาแสดงแต่ละหน้า/แต่ละเหรียญได้ 3–60 วินาที
- อัปโหลด ดูรายการ และลบ `profile.jpg` กับ `slide1.jpg`–`slide8.jpg` บน SD Card จากเว็บ
- ดาวน์โหลด Asset เริ่มต้นที่ยังขาดจาก GitHub ได้ โดยไม่เขียนทับรูปที่ผู้เรียนอัปโหลดเอง
- การตั้งค่าถูกเก็บใน Preferences ของ ESP32 จึงยังอยู่หลังรีสตาร์ต

ไม่มี Alarm, การแจ้งเตือนราคา, Telegram, LINE, MQTT, OTA หรือระบบผู้ใช้

## ข้อมูลออนไลน์

- ราคา: `GET https://api.bitkub.com/api/v3/market/ticker?sym=BTC_THB`
- กราฟ CDC: `GET https://api.bitkub.com/tradingview/history`
- CDC คำนวณ EMA 12 และ EMA 26 จากข้อมูล OHLC จำนวน 60 แท่ง
- BUY zone เมื่อ EMA 12 มากกว่า EMA 26; นอกนั้นเป็น SELL zone
- Public API ทั้งสองส่วนไม่ต้องใช้ API key

CDC ในโครงการนี้เป็นตัวอย่างเชิงการศึกษา ไม่ใช่คำแนะนำในการลงทุน

## ไลบรารี

- WiFiManager
- TFT_eSPI
- ArduinoJson 6
- JPEGDecoder
- WiFi, HTTPClient และ SD จาก ESP32 core

## เตรียม SD Card

หลัง Upload โค้ดครั้งแรกและตั้งค่า Wi-Fi สำเร็จ บอร์ดจะดาวน์โหลด `profile.jpg` และ `slide1.jpg` เริ่มต้นจาก GitHub ลง SD Card ให้อัตโนมัติ หากยังไม่มีไฟล์นั้นอยู่แล้ว แต่ยังควรใส่ SD Card แบบ FAT32 ก่อนเปิดบอร์ด

ใช้ SD Card แบบ FAT32 แล้วใส่ไฟล์ดังนี้:

```text
/
├── profile.jpg              JPEG 96x96 px
└── slides/
    ├── slide1.jpg           JPEG 320x240 px
    ├── slide2.jpg
    └── ... สูงสุด slide8.jpg
```

ชื่อไฟล์ต้องเรียงต่อกัน เริ่มจาก `slide1.jpg` และไม่มีเลขขาดกลาง

## จุดที่แก้ไขบ่อย

- `AppConfig.h`: ขา SD, เวลาแสดงหน้า, ข้อมูล Profile และค่า CDC
- `CoinData.cpp`: เหรียญและคู่ตลาด Bitkub
- `Pages.cpp`: รูปแบบหน้าจอทั้ง 5 สถานะ
- `BitkubApi.cpp`: ดึง JSON และคำนวณ CDC
- `CryptoClock_Learning.ino`: ควบคุมลำดับหน้าหลัก

## การ Compile

ทดสอบแล้วด้วย:

```bash
arduino-cli compile --fqbn esp32:esp32:esp32 CryptoClock_Learning
```

เปิด `CryptoClock_Learning.ino` แล้ว Compile ได้โดยไม่ต้องแก้ `TFT_eSPI/User_Setup.h` เพราะโปรเจกต์มี `tft_setup.h` สำหรับ ESP32-2432S028R อยู่แล้ว จากนั้น Upload ผ่านสาย USB Type-C Data
