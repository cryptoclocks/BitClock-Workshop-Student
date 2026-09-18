# HelloWiFi Starter

โปรเจกต์เริ่มต้นสำหรับ ESP32-2432S028R: เชื่อม Wi-Fi แบบกำหนดค่าในโค้ด และแสดง `Hello World` กลางจอ

1. เปิด `HelloWiFi_Starter.ino`
2. แก้ `WIFI_SSID` และ `WIFI_PASSWORD` ให้เป็น Wi-Fi 2.4 GHz ของตนเอง
3. ใน Arduino IDE เลือก **ESP32 Dev Module** และ Partition Scheme **Minimal SPIFFS (1.9MB APP with OTA)**
4. Upload ที่ 460800 baud
5. เปิด Serial Monitor ที่ 115200 baud เพื่อดู IP Address

ต้องตั้งค่าไลบรารี TFT_eSPI ให้ตรงกับบอร์ด ESP32-2432S028R ก่อน Upload เช่นเดียวกับโปรเจกต์ CryptoClock หลัก

คำสั่งที่ผู้เรียนจะได้ลอง:

- `const char*` เก็บข้อความ SSID และ Password
- `WiFi.begin()` เริ่มต่อ Wi-Fi
- `WiFi.status()` ตรวจสถานะ
- `Serial.println()` แสดงข้อความใน Serial Monitor
- `tft.drawString()` วาดข้อความบนจอ
- `setup()` ทำครั้งเดียวตอนเปิดเครื่อง
- `loop()` ทำซ้ำตลอดเวลา
