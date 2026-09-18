#ifndef GIF_DRAW_H
#define GIF_DRAW_H

#include <Arduino.h>
#include <AnimatedGIF.h>

// ปรับค่าตามไลบรารีจอที่ใช้ เช่น TFT_eSPI
#include <TFT_eSPI.h>
extern TFT_eSPI tft;

// ค่าคงที่หรือค่า offset ใช้ข้ามไฟล์ได้
extern int offsetX;
extern int offsetY;

// ฟังก์ชันหลักที่ AnimatedGIF จะเรียก
void GIFDraw(GIFDRAW *pDraw);

#endif // GIF_DRAW_H
