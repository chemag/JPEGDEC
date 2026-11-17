/*
 * SPDX-License-Identifier: Apache-2.0
 *
 * Decode 4:2:2 images whose last 16-pixel MCU column is clipped into exactly
 * sized framebuffers: a 20-pixel-wide image, and a 40-pixel-wide image
 * cropped to columns 16..39. JPEGPutMCU21() used to store all 16 pixels of
 * each row past the end of the line; clipping against the image width does
 * not catch the crop, where the MCU starts at output column 16 of a 24-pixel
 * line. Every block of the fixtures is identical, so clipped columns must
 * match the start of the row. Build with AddressSanitizer.
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "../src/JPEGDEC.h"
#include "crop_422_image.h"
#include "edge_422_image.h"

/* Decode into a malloc'ed width * height buffer of bpp-byte pixels. */
static uint8_t* Decode(int pixel_type, int bpp, int* width, int* height) {
  JPEGIMAGE jpeg;
  uint8_t* framebuffer;

  if (JPEGDEC_openRAM(&jpeg, (uint8_t*)edge_422_image,
                      (int)sizeof(edge_422_image), NULL) == 0) {
    fprintf(stderr, "open failed: %d\n", JPEGDEC_getLastError(&jpeg));
    return NULL;
  }
  *width = JPEGDEC_getWidth(&jpeg);
  *height = JPEGDEC_getHeight(&jpeg);
  if (JPEGDEC_getSubSample(&jpeg) != 0x21 || *width != 20 || *height != 8) {
    fprintf(stderr, "unexpected fixture\n");
    JPEGDEC_close(&jpeg);
    return NULL;
  }
  framebuffer = malloc((size_t)*width * (size_t)*height * (size_t)bpp);
  if (framebuffer == NULL) {
    JPEGDEC_close(&jpeg);
    return NULL;
  }
  JPEGDEC_setPixelType(&jpeg, pixel_type);
  JPEGDEC_setFramebuffer(&jpeg, framebuffer);
  if (JPEGDEC_decode(&jpeg, 0, 0, 0) == 0) {
    fprintf(stderr, "decode failed: %d\n", JPEGDEC_getLastError(&jpeg));
    free(framebuffer);
    framebuffer = NULL;
  }
  JPEGDEC_close(&jpeg);
  return framebuffer;
}

/* Columns 16..19 must equal columns 0..3 of the same row. */
static int ClippedColumnMatches(const uint8_t* framebuffer, int bpp, int width,
                                int height) {
  int x, y, b;

  for (y = 0; y < height; y++) {
    const uint8_t* row = framebuffer + (size_t)y * width * bpp;
    for (x = 0; x < 4; x++) {
      for (b = 0; b < bpp; b++) {
        if (row[(16 + x) * bpp + b] != row[x * bpp + b]) {
          fprintf(stderr, "bpp %d: pixel (%d,%d) differs from (%d,%d)\n", bpp,
                  16 + x, y, x, y);
          return 0;
        }
      }
    }
  }
  /* The rows differ, so a wrong-row read cannot match by accident. */
  for (b = 0; b < bpp; b++) {
    if (framebuffer[b] != framebuffer[(size_t)(height - 1) * width * bpp + b]) {
      return 1;
    }
  }
  fprintf(stderr, "fixture rows are identical\n");
  return 0;
}

/* Decode columns 16..39 of the 40x8 fixture; output columns 16..23 come
 * from the clipped MCU and must match output columns 0..7. */
static int CheckCrop(int pixel_type, int bpp) {
  JPEGIMAGE jpeg;
  uint8_t* framebuffer;
  int cx, cy, cw, ch, x, y, ok = 0;

  if (JPEGDEC_openRAM(&jpeg, (uint8_t*)crop_422_image,
                      (int)sizeof(crop_422_image), NULL) == 0) {
    return 0;
  }
  JPEGDEC_setCropArea(&jpeg, 16, 0, 24, 8);
  JPEGDEC_getCropArea(&jpeg, &cx, &cy, &cw, &ch);
  if (cx != 16 || cy != 0 || cw != 24 || ch != 8) {
    fprintf(stderr, "unexpected crop %d,%d %dx%d\n", cx, cy, cw, ch);
    JPEGDEC_close(&jpeg);
    return 0;
  }
  framebuffer = malloc((size_t)cw * (size_t)ch * (size_t)bpp);
  if (framebuffer != NULL) {
    JPEGDEC_setPixelType(&jpeg, pixel_type);
    JPEGDEC_setFramebuffer(&jpeg, framebuffer);
    ok = JPEGDEC_decode(&jpeg, 0, 0, 0) != 0;
  }
  JPEGDEC_close(&jpeg);
  for (y = 0; ok && y < ch; y++) {
    const uint8_t* row = framebuffer + (size_t)y * cw * bpp;
    for (x = 0; x < 8 * bpp; x++) {
      if (row[16 * bpp + x] != row[x]) {
        fprintf(stderr, "crop bpp %d: row %d byte %d differs\n", bpp, y, x);
        ok = 0;
        break;
      }
    }
  }
  free(framebuffer);
  return ok;
}

int main(void) {
  int width, height;
  uint8_t* rgb565 = Decode(JPEGD_PIXEL_RGB565_LE, 2, &width, &height);
  uint8_t* rgb8888 = Decode(JPEGD_PIXEL_RGB8888, 4, &width, &height);
  int result = 1;

  if (rgb565 != NULL && rgb8888 != NULL &&
      ClippedColumnMatches(rgb565, 2, width, height) &&
      ClippedColumnMatches(rgb8888, 4, width, height) &&
      CheckCrop(JPEGD_PIXEL_RGB565_LE, 2) &&
      CheckCrop(JPEGD_PIXEL_RGB8888, 4)) {
    result = 0;
  }

  free(rgb565);
  free(rgb8888);
  if (result == 0)
    printf("Clipped 4:2:2 MCUs stay in bounds and use the right rows\n");
  return result;
}
