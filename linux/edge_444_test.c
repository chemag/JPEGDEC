/*
 * SPDX-License-Identifier: Apache-2.0
 *
 * Decode a 20-pixel-wide 4:4:4 image into an exactly sized framebuffer. The
 * last MCU column is clipped to 4 pixels; the SSE path used to store all 8
 * pixels of each row past the end of the line, and the C path did not skip
 * the unused luma samples, so rows after the first read the wrong row. Every
 * block of the fixture is identical, so the clipped column must match the
 * first 4 columns. Build with AddressSanitizer.
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "../src/JPEGDEC.h"
#include "edge_444_image.h"

/* Decode into a malloc'ed width * height buffer of bpp-byte pixels. */
static uint8_t* Decode(int pixel_type, int bpp, int* width, int* height) {
  JPEGIMAGE jpeg;
  uint8_t* framebuffer;

  if (JPEGDEC_openRAM(&jpeg, (uint8_t*)edge_444_image,
                      (int)sizeof(edge_444_image), NULL) == 0) {
    fprintf(stderr, "open failed: %d\n", JPEGDEC_getLastError(&jpeg));
    return NULL;
  }
  *width = JPEGDEC_getWidth(&jpeg);
  *height = JPEGDEC_getHeight(&jpeg);
  if (JPEGDEC_getSubSample(&jpeg) != 0x11 || *width != 20 || *height != 8) {
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

int main(void) {
  int width, height;
  uint8_t* rgb565 = Decode(JPEGD_PIXEL_RGB565_LE, 2, &width, &height);
  uint8_t* rgb8888 = Decode(JPEGD_PIXEL_RGB8888, 4, &width, &height);
  int result = 1;

  if (rgb565 != NULL && rgb8888 != NULL &&
      ClippedColumnMatches(rgb565, 2, width, height) &&
      ClippedColumnMatches(rgb8888, 4, width, height)) {
    result = 0;
  }

  free(rgb565);
  free(rgb8888);
  if (result == 0)
    printf("Clipped 4:4:4 MCUs stay in bounds and use the right rows\n");
  return result;
}
