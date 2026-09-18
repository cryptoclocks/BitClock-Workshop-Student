# เริ่มต้นสำหรับนักเรียน

แพ็กเกจนี้รวมไลบรารีที่จำเป็นไว้แล้ว ไม่ต้องดาวน์โหลดไลบรารีเพิ่มเอง

1. แตกไฟล์ ZIP แล้วดับเบิลคลิก `Install_Libraries.command` หนึ่งครั้ง
2. เปิด Arduino IDE 2.x แล้วติดตั้งบอร์ด **esp32 by Espressif Systems เวอร์ชัน 2.0.17** จาก Boards Manager หากเครื่องยังไม่มี
3. เปิด `CryptoClock_Learning.ino` แล้วเลือก **Tools → Board → ESP32 Arduino → ESP32 Dev Module**
4. เลือกพอร์ต USB ของบอร์ด และตั้ง **Upload Speed: 460800** กับ **Partition Scheme: Minimal SPIFFS (Large APPS with OTA)**
5. กด Verify ก่อน แล้วจึง Upload

สำหรับแบบฝึกเริ่มต้น ให้เปิด `HelloWiFi_Starter/HelloWiFi_Starter.ino` ได้เลย โดยใช้ Board และ Port เดียวกัน

## ไลบรารีที่รวมมา

`TFT_eSPI`, `ArduinoJson`, `AESLib`, `JPEGDecoder`, `JPEGDEC`, `XPT2046_Bitbang` และ `AnimatedGIF` อยู่ในโฟลเดอร์ `bundled-libraries/` พร้อมไฟล์ต้นฉบับและ license ของแต่ละไลบรารี

> หมายเหตุ: ESP32 board package เป็น toolchain ของบอร์ด ไม่ใช่ไลบรารี จึงติดตั้งผ่าน Boards Manager เพียงครั้งเดียวต่อเครื่อง
