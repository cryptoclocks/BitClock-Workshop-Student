# เริ่มต้นสำหรับนักเรียน

แพ็กเกจนี้รวมโค้ด เว็บไซต์สอน ไฟล์ SD Card และไลบรารีที่จำเป็นไว้แล้ว ไม่ต้องดาวน์โหลดไลบรารีเพิ่มเอง

1. ดาวน์โหลดและแตกไฟล์ ZIP จาก [BitClock Workshop Student](https://github.com/cryptoclocks/BitClock-Workshop-Student/archive/refs/heads/main.zip)
2. ติดตั้ง [Arduino IDE 2.x](https://www.arduino.cc/en/software) หากยังไม่มี
3. ติดตั้งไลบรารีที่รวมในแพ็กเกจหนึ่งครั้ง
   - **macOS:** ดับเบิลคลิก `Install_Libraries.command` (หาก macOS เตือน ให้คลิกขวาแล้วเลือก Open)
   - **Windows 10/11:** ดับเบิลคลิก `Install_Libraries.bat` แล้วรอจนขึ้นข้อความเสร็จสิ้น
4. เปิด Arduino IDE 2.x แล้วติดตั้งบอร์ด **esp32 by Espressif Systems เวอร์ชัน 2.0.17** จาก Boards Manager หากเครื่องยังไม่มี
5. เปิด `CryptoClock_Learning.ino` แล้วเลือก **Tools → Board → ESP32 Arduino → ESP32 Dev Module**
6. เลือกพอร์ต USB ของบอร์ด และตั้ง **Upload Speed: 460800** กับ **Partition Scheme: Minimal SPIFFS (1.9MB APP / 190KB SPIFFS)**
7. กด Verify ก่อน แล้วจึง Upload

สำหรับแบบฝึกเริ่มต้น ให้เปิด `HelloWiFi_Starter/HelloWiFi_Starter.ino` ได้เลย โดยใช้ Board และ Port เดียวกัน โค้ดตัวอย่างตั้งค่า `tft.setSwapBytes(true)` ไว้แล้วให้ตรงกับจอ ESP32-2432S028R

## ไฟล์ในแพ็กเกจ

- `CryptoClock_Learning.ino` — โค้ดหลักของ CryptoClock
- `HelloWiFi_Starter/` — แบบฝึกเริ่มต้น ต่อ Wi-Fi แล้วพิมพ์ข้อความกลางจอ
- `sd/` — ไฟล์ระบบและรูปตัวอย่างสำหรับ MicroSD Card
- `teaching-site/index.html` — เว็บไซต์บทเรียน เปิดด้วยเบราว์เซอร์ได้โดยไม่ต้องติดตั้งเพิ่ม
- `bundled-libraries/` — ไลบรารีที่ตัวติดตั้งคัดลอกให้ Arduino IDE

ก่อนใช้งานจริง ให้คัดลอกเนื้อหาใน `sd/` ลง MicroSD ที่ฟอร์แมต FAT32 แล้วใส่การ์ดเข้าบอร์ด

## ไลบรารีที่รวมมา

`TFT_eSPI`, `ArduinoJson`, `AESLib`, `JPEGDecoder`, `JPEGDEC`, `XPT2046_Bitbang` และ `AnimatedGIF` อยู่ในโฟลเดอร์ `bundled-libraries/` พร้อมไฟล์ต้นฉบับและ license ของแต่ละไลบรารี

> หมายเหตุ: ESP32 board package เป็น toolchain ของบอร์ด ไม่ใช่ไลบรารี จึงติดตั้งผ่าน Boards Manager เพียงครั้งเดียวต่อเครื่อง
