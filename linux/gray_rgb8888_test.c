/*
 * SPDX-License-Identifier: Apache-2.0
 *
 * Decode a grayscale image to RGB8888 at every scale. JPEGPutMCUGray() only
 * wrote RGB565, so RGB8888 output of grayscale images held 16-bit pixels
 * packed two per 32-bit slot. Each RGB8888 pixel must be gray, opaque, and
 * agree with the RGB565 output. Build with AddressSanitizer.
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "../src/JPEGDEC.h"
#include "gray_image.h"

/* Decode into a framebuffer of the scaled image size, in whole MCUs. */
static void* Decode(int pixel_type, int bpp, int options, int shift, int* w,
                    int* h) {
  JPEGIMAGE jpeg;
  void* framebuffer = NULL;
  size_t padded_w, padded_h;

  if (JPEGDEC_openRAM(&jpeg, (uint8_t*)gray_image, (int)sizeof(gray_image),
                      NULL) == 0) {
    return NULL;
  }
  *w = (JPEGDEC_getWidth(&jpeg) + (1 << shift) - 1) >> shift;
  *h = (JPEGDEC_getHeight(&jpeg) + (1 << shift) - 1) >> shift;
  padded_w = (((size_t)JPEGDEC_getWidth(&jpeg) + 15) / 16 * 16) >> shift;
  padded_h = (((size_t)JPEGDEC_getHeight(&jpeg) + 15) / 16 * 16) >> shift;
  framebuffer = calloc(padded_w * padded_h, (size_t)bpp);
  if (framebuffer != NULL) {
    JPEGDEC_setPixelType(&jpeg, pixel_type);
    JPEGDEC_setFramebuffer(&jpeg, framebuffer);
    if (JPEGDEC_decode(&jpeg, 0, 0, options) == 0) {
      free(framebuffer);
      framebuffer = NULL;
    }
  }
  JPEGDEC_close(&jpeg);
  return framebuffer;
}

int main(void) {
  static const int kOptions[] = {0, JPEG_SCALE_HALF, JPEG_SCALE_QUARTER,
                                 JPEG_SCALE_EIGHTH};
  int shift, ok = 1;

  for (shift = 0; shift < 4; shift++) {
    int w, h, w2, h2, i;
    uint16_t* rgb565 =
        Decode(JPEGD_PIXEL_RGB565_LE, 2, kOptions[shift], shift, &w, &h);
    uint8_t* rgb8888 =
        Decode(JPEGD_PIXEL_RGB8888, 4, kOptions[shift], shift, &w2, &h2);

    if (rgb565 == NULL || rgb8888 == NULL) {
      fprintf(stderr, "shift %d: decode failed\n", shift);
      ok = 0;
    } else {
      for (i = 0; i < w * h; i++) {
        const uint8_t* p = rgb8888 + (size_t)i * 4;
        int g = (rgb565[i] >> 5) & 63;
        if (p[0] != p[1] || p[1] != p[2] || p[3] != 0xff ||
            abs((p[1] >> 2) - g) > 1) {
          fprintf(stderr, "shift %d: pixel %d RGBA %02x%02x%02x%02x vs %04x\n",
                  shift, i, p[0], p[1], p[2], p[3], rgb565[i]);
          ok = 0;
          break;
        }
      }
    }
    free(rgb565);
    free(rgb8888);
  }
  if (!ok) return 1;
  printf("Grayscale RGB8888 output is gray, opaque, and matches RGB565\n");
  return 0;
}
