# Bootstrap Assets

ไฟล์ในโฟลเดอร์นี้คือรูปเริ่มต้นที่ ESP32 ดาวน์โหลดลง SD Card หลังตั้งค่า Wi-Fi ครั้งแรก

- `profile.jpg` → `/profile.jpg`
- `slides/slide1.jpg` → `/slides/slide1.jpg`

หากเปลี่ยนรูป ต้องคำนวณ SHA-256 ใหม่และแก้ค่า checksum ใน `BootstrapAssets.cpp` ก่อนใช้งาน เพื่อให้บอร์ดยอมรับไฟล์ใหม่อย่างปลอดภัย
