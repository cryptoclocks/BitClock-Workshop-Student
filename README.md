# CryptoClock V4.1.4 CDC (workshop copy)

The active Arduino sketch is `CryptoClock_Learning.ino`. It is a copy of the original `V414_CDC` firmware, with Google Apps Script registration, profile upload, and permission check removed. The original files at `/Users/natthapongsuwanjit/Desktop/CryptoClockV3/Arduino/Version/V414_CDC` are unchanged.

The workshop build includes coin logos, SD slides, a local settings website, Wi-Fi setup, price APIs, CDC Action Zone, audio, and LittleFS settings.

When a device has no registered ID in LittleFS, this build creates a stable local ID from the ESP32 Wi-Fi MAC address, prefixed with `CCN`. An existing stored device ID is kept. Services that expect a previously registered server-side device ID may still require configuration on that service; removing Google Scripts does not replace server-side registration.

## Build and flash

Board: **ESP32 Dev Module** (ESP32 Arduino core 2.0.17). Partition: **Minimal SPIFFS (1.9MB APP / 190KB filesystem)**. Upload speed: **460800**. Required third-party libraries are the same ones used by the V414_CDC original.

```sh
arduino-cli compile --fqbn 'esp32:esp32:esp32:PartitionScheme=min_spiffs' .
arduino-cli upload --fqbn 'esp32:esp32:esp32:PartitionScheme=min_spiffs,UploadSpeed=460800' -p /dev/cu.usbserial-120 .
```

The `sd/` directory is an image of the original microSD contents. Copy its **contents**, including `4.1.4/`, to the root of a FAT32 microSD card. LittleFS settings are created on first boot if missing; `data/` is a local backup of the original project's filesystem data and is deliberately excluded from Git because it can contain device and Wi-Fi credentials. When no device ID exists, the firmware creates a stable `CCN...` ID from the ESP32 MAC address.

The local teaching site documents the workshop flow and is kept aligned with this firmware.
# CryptoClock Learning Workshop

เริ่มต้นที่ [STUDENT-START-HERE.md](STUDENT-START-HERE.md) — ไลบรารีทั้งหมดที่สเก็ตช์ต้องใช้รวมอยู่ในแพ็กเกจแล้ว
