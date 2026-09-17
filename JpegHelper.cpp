#include "JpegHelper.h"
#include <SD.h>
#include <JPEGDecoder.h>

static void renderJpeg(TFT_eSPI& screen, int xOffset, int yOffset) {
  uint16_t* pixels;
  uint16_t mcuWidth = JpegDec.MCUWidth;
  uint16_t mcuHeight = JpegDec.MCUHeight;
  uint32_t maxX = JpegDec.width + xOffset;
  uint32_t maxY = JpegDec.height + yOffset;
  bool oldSwap = screen.getSwapBytes();
  screen.setSwapBytes(true);

  while (JpegDec.read()) {
    pixels = JpegDec.pImage;
    int x = JpegDec.MCUx * mcuWidth + xOffset;
    int y = JpegDec.MCUy * mcuHeight + yOffset;
    uint16_t width = (x + mcuWidth <= (int)maxX) ? mcuWidth : maxX - x;
    uint16_t height = (y + mcuHeight <= (int)maxY) ? mcuHeight : maxY - y;

    if (width != mcuWidth) {
      uint16_t* compact = pixels + width;
      for (uint16_t row = 1; row < height; row++) {
        uint32_t source = row * mcuWidth;
        for (uint16_t col = 0; col < width; col++) *compact++ = pixels[source + col];
      }
    }

    if (x < screen.width() && y < screen.height()) {
      uint16_t drawWidth = min<uint16_t>(width, screen.width() - x);
      uint16_t drawHeight = min<uint16_t>(height, screen.height() - y);
      screen.pushImage(x, y, drawWidth, drawHeight, pixels);
    } else {
      JpegDec.abort();
    }
  }
  screen.setSwapBytes(oldSwap);
}

bool drawSdJpeg(TFT_eSPI& screen, const char* filename, int x, int y) {
  File jpegFile = SD.open(filename, FILE_READ);
  if (!jpegFile) {
    Serial.printf("JPEG not found: %s\n", filename);
    return false;
  }

  bool decoded = JpegDec.decodeSdFile(jpegFile);
  if (!decoded) {
    Serial.printf("JPEG decode failed: %s\n", filename);
    return false;
  }

  renderJpeg(screen, x, y);
  return true;
}
