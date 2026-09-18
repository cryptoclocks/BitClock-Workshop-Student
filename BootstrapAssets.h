#pragma once

#include <Arduino.h>

// ดาวน์โหลดเฉพาะ Asset เริ่มต้นที่ยังไม่มีบน SD Card
// ใช้ checksum ที่ฝังในโค้ดเพื่อตรวจว่าไฟล์ที่ได้ตรงกับชุดใน GitHub
bool downloadMissingBootstrapAssets(String& summary);
