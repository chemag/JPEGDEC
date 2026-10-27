/*
 * SPDX-License-Identifier: Apache-2.0
 *
 * Clamp crop areas that extend past the image. setCropArea() used to shrink
 * an overlong width to the image width minus one MCU, ignoring the origin,
 * so the area could still end far past the image edge, and the framebuffer
 * pitch followed it. For images smaller than one MCU the origin went
 * negative. Build with AddressSanitizer.
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "../src/JPEGDEC.h"
#include "../test_images/tulips.h"
#include "crop_small_image.h"

/* Returns 1 if the clamped area fits the image and decodes into a framebuffer
 * of the clamped size, rounded up to whole 16x16 MCUs as the framebuffer path
 * requires. */
static int Check(const char* name, const uint8_t* data, int size, int x, int y,
                 int w, int h) {
  JPEGIMAGE jpeg;
  int cx, cy, cw, ch, width, height, ok = 0;
  uint16_t* framebuffer = NULL;

  if (JPEGDEC_openRAM(&jpeg, (uint8_t*)data, size, NULL) == 0) {
    fprintf(stderr, "%s: open failed: %d\n", name, JPEGDEC_getLastError(&jpeg));
    return 0;
  }
  width = JPEGDEC_getWidth(&jpeg);
  height = JPEGDEC_getHeight(&jpeg);
  JPEGDEC_setCropArea(&jpeg, x, y, w, h);
  JPEGDEC_getCropArea(&jpeg, &cx, &cy, &cw, &ch);
  if (cx < 0 || cy < 0 || cw < 1 || ch < 1 || cx + cw > width ||
      cy + ch > height) {
    fprintf(stderr, "%s: crop (%d,%d %dx%d) outside %dx%d image\n", name, cx,
            cy, cw, ch, width, height);
  } else if ((framebuffer = malloc(((size_t)cw + 15) / 16 * 16 *
                                   (((size_t)ch + 15) / 16 * 16) * 2U)) ==
             NULL) {
    fprintf(stderr, "%s: allocation failed\n", name);
  } else {
    JPEGDEC_setPixelType(&jpeg, JPEGD_PIXEL_RGB565_LE);
    JPEGDEC_setFramebuffer(&jpeg, framebuffer);
    if (JPEGDEC_decode(&jpeg, 0, 0, 0) == 0) {
      fprintf(stderr, "%s: decode failed: %d\n", name,
              JPEGDEC_getLastError(&jpeg));
    } else {
      ok = 1;
    }
  }
  JPEGDEC_close(&jpeg);
  free(framebuffer);
  return ok;
}

int main(void) {
  /* 640x480 4:2:0: a request reaching past the bottom-right corner. */
  if (!Check("tulips", tulips, (int)sizeof(tulips), 630, 470, 640, 480) ||
      /* 8x8 4:2:0, smaller than one 16x16 MCU. */
      !Check("small", crop_small_image, (int)sizeof(crop_small_image), 4, 4, 8,
             8)) {
    return 1;
  }
  printf("Overlong crop areas clamped to the image\n");
  return 0;
}
