#include "gif_draw.h"

#define DISPLAY_WIDTH  tft.width()
#define DISPLAY_HEIGHT tft.height()
#define BUFFER_SIZE 256

uint16_t usTemp[1][BUFFER_SIZE];
bool dmaBuf = 0;

int offsetX = 112;
int offsetY = 72;

void GIFDraw(GIFDRAW *pDraw)
{
  uint8_t *s;
  uint16_t *usPalette;
  int x, y, iWidth, iCount;

  iWidth = pDraw->iWidth;
  if (iWidth + pDraw->iX + offsetX > DISPLAY_WIDTH)
    iWidth = DISPLAY_WIDTH - (pDraw->iX + offsetX);

  usPalette = pDraw->pPalette;
  y = pDraw->iY + pDraw->y + offsetY;
  if (y >= DISPLAY_HEIGHT || (pDraw->iX + offsetX) >= DISPLAY_WIDTH || iWidth < 1)
    return;

  s = pDraw->pPixels;
  if (pDraw->ucDisposalMethod == 2) {
    for (x = 0; x < iWidth; x++) {
      if (s[x] == pDraw->ucTransparent)
        s[x] = pDraw->ucBackground;
    }
    pDraw->ucHasTransparency = 0;
  }

  if (pDraw->ucHasTransparency) {
    uint8_t *pEnd, c, ucTransparent = pDraw->ucTransparent;
    uint16_t color;
    pEnd = s + iWidth;
    x = 0;
    iCount = 0;

    while (x < iWidth) {
      c = ucTransparent - 1;
      uint16_t *d = &usTemp[0][0];
      while (c != ucTransparent && s < pEnd && iCount < BUFFER_SIZE) {
        c = *s++;
        if (c == ucTransparent)
          s--;
        else {
          color = usPalette[c];
          *d++ = (color >> 8) | (color << 8);
          iCount++;
        }
      }
      if (iCount) {
        if (pDraw->iX + x + offsetX < DISPLAY_WIDTH && y < DISPLAY_HEIGHT) {
          tft.setAddrWindow(pDraw->iX + x + offsetX, y, iCount, 1);
          tft.pushPixels(usTemp, iCount);
        }
        x += iCount;
        iCount = 0;
      }
      c = ucTransparent;
      while (c == ucTransparent && s < pEnd) {
        c = *s++;
        if (c == ucTransparent)
          x++;
        else
          s--;
      }
    }
  } else {
    s = pDraw->pPixels;
    uint16_t color;

    if (iWidth <= BUFFER_SIZE) {
      for (iCount = 0; iCount < iWidth; iCount++) {
        color = usPalette[*s++];
        usTemp[dmaBuf][iCount] = (color >> 8) | (color << 8);
      }
    } else {
      for (iCount = 0; iCount < BUFFER_SIZE; iCount++) {
        color = usPalette[*s++];
        usTemp[dmaBuf][iCount] = (color >> 8) | (color << 8);
      }
    }

    #ifdef USE_DMA
      tft.dmaWait();
      tft.setAddrWindow(pDraw->iX + offsetX, y, iCount, 1);
      tft.pushPixelsDMA(&usTemp[dmaBuf][0], iCount);
      dmaBuf = !dmaBuf;
    #else
      tft.setAddrWindow(pDraw->iX + offsetX, y, iCount, 1);
      tft.pushPixels(&usTemp[0][0], iCount);
    #endif

    iWidth -= iCount;
    while (iWidth > 0) {
      if (iWidth <= BUFFER_SIZE) {
        for (iCount = 0; iCount < iWidth; iCount++) {
          color = usPalette[*s++];
          usTemp[dmaBuf][iCount] = (color >> 8) | (color << 8);
        }
      } else {
        for (iCount = 0; iCount < BUFFER_SIZE; iCount++) {
          color = usPalette[*s++];
          usTemp[dmaBuf][iCount] = (color >> 8) | (color << 8);
        }
      }
      tft.pushPixels(&usTemp[0][0], iCount);
      iWidth -= iCount;
    }
  }
}
