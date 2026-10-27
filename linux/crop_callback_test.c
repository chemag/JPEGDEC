/*
 * SPDX-License-Identifier: Apache-2.0
 *
 * Deliver every pixel of a crop area to the block callback exactly once.
 * When a crop area did not start at column 0 and was wider than one callback
 * block, DecodeJPEG() compared an output column with a source column to size
 * the last block of each row, never drew it, and dropped the right part of
 * the area. Build with AddressSanitizer.
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../src/JPEGDEC.h"
#include "../test_images/tulips.h"

static uint8_t* g_seen;
static int g_width, g_height, g_bad;

static int Draw(JPEGDRAW* draw) {
  int x, y;

  if (draw->iWidthUsed < 0 || draw->iWidthUsed > draw->iWidth || draw->x < 0 ||
      draw->y < 0 || draw->x + draw->iWidthUsed > g_width ||
      draw->y + draw->iHeight > g_height) {
    g_bad++;
    return 1;
  }
  for (y = 0; y < draw->iHeight; y++) {
    for (x = 0; x < draw->iWidthUsed; x++) {
      g_seen[(size_t)(draw->y + y) * g_width + draw->x + x]++;
    }
  }
  return 1;
}

static int Check(int x, int y, int w, int h) {
  JPEGIMAGE jpeg;
  int cx, cy, i, ok = 1;

  if (JPEGDEC_openRAM(&jpeg, (uint8_t*)tulips, (int)sizeof(tulips), Draw) ==
      0) {
    return 0;
  }
  JPEGDEC_setPixelType(&jpeg, JPEGD_PIXEL_RGB565_LE);
  JPEGDEC_setCropArea(&jpeg, x, y, w, h);
  JPEGDEC_getCropArea(&jpeg, &cx, &cy, &g_width, &g_height);
  g_seen = calloc((size_t)g_width * (size_t)g_height, 1);
  g_bad = 0;
  if (g_seen == NULL || JPEGDEC_decode(&jpeg, 0, 0, 0) == 0) {
    ok = 0;
  } else {
    for (i = 0; i < g_width * g_height; i++) {
      if (g_seen[i] != 1) {
        fprintf(stderr, "crop %d,%d %dx%d: pixel (%d,%d) drawn %d times\n", cx,
                cy, g_width, g_height, i % g_width, i / g_width, g_seen[i]);
        ok = 0;
        break;
      }
    }
    if (g_bad != 0) {
      fprintf(stderr, "crop %d,%d %dx%d: %d blocks out of range\n", cx, cy,
              g_width, g_height, g_bad);
      ok = 0;
    }
  }
  JPEGDEC_close(&jpeg);
  free(g_seen);
  return ok;
}

int main(void) {
  /* Wider than one callback block, starting past column 0. */
  if (!Check(320, 240, 160, 160) || !Check(33, 17, 300, 31) ||
      /* Starting at column 0, and reaching the right edge of the image. */
      !Check(0, 0, 300, 40) || !Check(600, 440, 64, 64)) {
    return 1;
  }
  printf("Crop areas delivered to the callback exactly once\n");
  return 0;
}
