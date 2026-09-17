# ไฟล์ที่ต้องคัดลอกลง SD Card

โครงสร้างปลายทางบน SD Card:

```text
/
├── profile.jpg
└── slides/
    ├── slide1.jpg
    ├── slide2.jpg
    ├── slide3.jpg
    └── ... สูงสุด slide8.jpg
```

- `profile.jpg`: JPEG ขนาด 96x96 px
- `slide1.jpg` เป็นต้นไป: JPEG ขนาด 320x240 px
- ฟอร์แมต SD Card เป็น FAT32
- ตั้งชื่อด้วยตัวพิมพ์เล็กตามตัวอย่าง
- หมายเลขภาพต้องต่อกัน หากไม่มี `slide2.jpg` โปรแกรมจะไม่นับภาพหลังจากนั้น
