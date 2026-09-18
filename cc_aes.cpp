#include <AESLib.h>

AESLib aesLib;
String key = "ClocktoCrypt1234"; // AES-128 Key (16 bytes)
String iv = "ClocktoCrypt1234";  // Initialization Vector (16 bytes)


// ฟังก์ชันสำหรับแปลง Hex เป็น Binary
void hexToBin(const char* hex, byte* bin, size_t binSize) {
  for (size_t i = 0; i < binSize; i++) {
    sscanf(&hex[i * 2], "%2hhx", &bin[i]);
  }
}

// ฟังก์ชันสำหรับแปลง Binary เป็น Hex
String binToHex(const byte* bin, size_t len) {
  String hex = "";
  for (size_t i = 0; i < len; i++) {
    if (bin[i] < 0x10) hex += "0"; // เติม '0' ถ้าค่าน้อยกว่า 0x10
    hex += String(bin[i], HEX);
  }
  return hex;
}

String aesEncrypt(String plainText) {
  byte encrypted[512];       // Buffer สำหรับเก็บข้อมูลที่เข้ารหัส
  byte aesKey[16];           // Key (16 bytes)
  byte aesIV[16];            // IV (16 bytes)

  // รีเซ็ต Key และ IV
  memcpy(aesKey, key.c_str(), 16);
  memcpy(aesIV, iv.c_str(), 16);


  aesLib.set_paddingmode((paddingMode)1);
  // เข้ารหัส
  uint16_t encryptedLength = aesLib.encrypt(
                               (byte*)plainText.c_str(),
                               plainText.length(),
                               encrypted,
                               aesKey,
                               128,
                               aesIV
                             );

  // ตรวจสอบความยาวข้อมูลที่เข้ารหัส
  if (encryptedLength == 0) {
    Serial.println("Encryption failed!");
    return "";
  }

  // แปลงผลลัพธ์เป็น Hex
  return binToHex(encrypted, encryptedLength);
}




// ฟังก์ชัน Decrypt
// ฟังก์ชัน Decrypt
String aesDecrypt(String inputHex) {
  if (inputHex.length() > 1024) {
    Serial.println("Input Hex too large!");
    return "";
  }

  byte encrypted[512];       // Buffer สำหรับเก็บข้อมูลที่เข้ารหัส
  byte decrypted[512];       // Buffer สำหรับเก็บผลลัพธ์ที่ถอดรหัส
  byte aesKey[16];           // Key (16 bytes)
  byte aesIV[16];            // IV (16 bytes)

  // แปลง Key และ IV เป็น Byte Array
  memcpy(aesKey, key.c_str(), 16);
  memcpy(aesIV, iv.c_str(), 16);

  // แปลง Input Hex เป็น Byte Array
  size_t inputLen = inputHex.length() / 2; // ขนาดข้อมูลที่ถอดรหัส
  hexToBin(inputHex.c_str(), encrypted, inputLen);

  // Decrypt
  uint16_t decryptedLength = aesLib.decrypt(
                               encrypted,     // ข้อมูลเข้ารหัส
                               inputLen,      // ความยาวของข้อมูลที่เข้ารหัส
                               decrypted,     // Buffer เก็บผลลัพธ์
                               aesKey,        // Key
                               128,           // ขนาดบิตของ Key (128-bit)
                               aesIV          // Initialization Vector
                             );

  if (decryptedLength == 0) {
    Serial.println("Decryption failed!");
    return "";
  }

  // เพิ่ม '\0' เพื่อทำให้ buffer เป็น String ที่สิ้นสุดด้วย null
  decrypted[decryptedLength] = '\0';

  String a =  String((char*)decrypted) ;
  a.trim();
  // คืนค่า String ผลลัพธ์
  return a;
}
