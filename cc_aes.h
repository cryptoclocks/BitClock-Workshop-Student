#include <Arduino.h>
#include <AESLib.h> // ไลบรารี AES

extern AESLib aesLib;
extern String key;
extern String iv;

String aesEncrypt(String plainText);
String aesDecrypt(String inputHex);
