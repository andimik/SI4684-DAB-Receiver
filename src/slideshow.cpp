#ifndef slideshow_cpp
#define slideshow_cpp

#include "slideshow.h"

File pngfile;
File jpgfile;
PNG png;

void ShowSlideShow(void) {
  if (radio.isJPG) {
    tft.fillScreen(TFT_BLACK);
    jpgfile = LittleFS.open("/slideshow.img", "rb");
    if (!jpgfile) {
      return;
    }
    bool decoded = JpegDec.decodeFsFile(jpgfile);
    if (decoded) {
      uint16_t *pImg;
      uint16_t mcu_w = JpegDec.MCUWidth;
      uint16_t mcu_h = JpegDec.MCUHeight;
      uint32_t max_x = JpegDec.width;
      uint32_t max_y = JpegDec.height;

      while (JpegDec.read()) {
        pImg = JpegDec.pImage;
        int mcu_x = JpegDec.MCUx * mcu_w;
        int mcu_y = JpegDec.MCUy * mcu_h;
        uint32_t win_w = mcu_w;
        uint32_t win_h = mcu_h;

        if (mcu_x + mcu_w > max_x) win_w = max_x - mcu_x;
        if (mcu_y + mcu_h > max_y) win_h = max_y - mcu_y;

        tft.startWrite();
        tft.setAddrWindow(mcu_x + (320 - JpegDec.width) / 2, mcu_y + (240 - JpegDec.height) / 2, win_w, win_h);

        for (int h = 0; h < win_h; h++) {
          for (int w = 0; w < win_w; w++) {
            tft.pushColor(*pImg++);
          }
        }
        tft.endWrite();
      }
    }
    jpgfile.close();
  } else if (radio.isPNG) {
    tft.fillScreen(TFT_BLACK);
    String filename = "/slideshow.img";
    pngfile = LittleFS.open(filename.c_str(), "rb");

    if (pngfile) {
      int16_t rc = png.open(filename.c_str(),
      [&pngfile](const char *filename, int32_t *size) -> void * {
        *size = pngfile.size();
        return &pngfile;
      },
      [&pngfile](void *handle) {
      },
      [&pngfile](PNGFILE * page, uint8_t *buffer, int32_t length) -> int32_t {
        if (!pngfile || !pngfile.available()) return 0;
        return pngfile.read(buffer, length);
      },
      [&pngfile](PNGFILE * page, int32_t position) -> int32_t {
        if (!pngfile || !pngfile.available()) return 0;
        return pngfile.seek(position);
      },
      [&pngfile, &tft](PNGDRAW * pDraw) {
        uint16_t lineBuffer[320];
        png.getLineAsRGB565(pDraw, lineBuffer, PNG_RGB565_LITTLE_ENDIAN, 0xffffffff);
        tft.pushImage((320 - png.getWidth()) / 2, ((240 - png.getHeight()) / 2) + pDraw->y, pDraw->iWidth, 1, lineBuffer);
      });

      if (rc == PNG_SUCCESS) {
        tft.startWrite();
        rc = png.decode(nullptr, 0);
        png.close();
        tft.endWrite();
      }
    }
  }
}
#endif