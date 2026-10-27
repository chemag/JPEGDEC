/*
 * SPDX-License-Identifier: Apache-2.0
 *
 * Crop first, then scale: a crop area in source pixels decoded at 1/2, 1/4,
 * or 1/8 scale must equal the matching rectangle of the whole image decoded
 * at the same scale, through both the block callback and a framebuffer.
 * Progressive images always decode at 1/8. DecodeJPEG() used to compare
 * scaled MCU positions with the source crop area and use the source crop
 * width as the scaled output width, so callbacks got negative coordinates
 * and framebuffers were written out of bounds. Build with AddressSanitizer.
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../src/JPEGDEC.h"
#include "../test_images/tulips.h"
#include "../test_images/zebra.h"
#include "progressive_test_image.h"

typedef struct {
  uint16_t* pixels;
  uint8_t* seen;
  int width, height, bad;
} Canvas;

static Canvas g_canvas;

static int Draw(JPEGDRAW* draw) {
  const uint16_t* src = (const uint16_t*)draw->pPixels;
  int x, y;

  if (draw->iWidthUsed < 0 || draw->iWidthUsed > draw->iWidth || draw->x < 0 ||
      draw->y < 0 || draw->x + draw->iWidthUsed > g_canvas.width ||
      draw->y + draw->iHeight > g_canvas.height) {
    g_canvas.bad++;
    return 1;
  }
  for (y = 0; y < draw->iHeight; y++) {
    for (x = 0; x < draw->iWidthUsed; x++) {
      size_t i = (size_t)(draw->y + y) * g_canvas.width + draw->x + x;
      g_canvas.pixels[i] = src[y * draw->iWidth + x];
      g_canvas.seen[i]++;
    }
  }
  return 1;
}

/* Scale shift that DecodeJPEG() applies for these options. */
static int ScaleShift(JPEGIMAGE* jpeg, int options) {
  if (JPEGDEC_getJPEGType(jpeg) == JPEG_MODE_PROGRESSIVE) return 3;
  if (options & JPEG_SCALE_HALF) return 1;
  if (options & JPEG_SCALE_QUARTER) return 2;
  if (options & JPEG_SCALE_EIGHTH) return 3;
  return 0;
}

/* Decode through the callback into g_canvas; crop[2] == 0 means no crop.
 * On success crop[] holds the stored crop area and *shift the scale. */
static int DecodeCallback(const uint8_t* data, int size, int options,
                          int crop[4], int* shift) {
  JPEGIMAGE jpeg;
  int ok = 0, i;

  memset(&g_canvas, 0, sizeof(g_canvas));
  if (JPEGDEC_openRAM(&jpeg, (uint8_t*)data, size, Draw) == 0) return 0;
  JPEGDEC_setPixelType(&jpeg, JPEGD_PIXEL_RGB565_LE);
  if (crop[2] != 0)
    JPEGDEC_setCropArea(&jpeg, crop[0], crop[1], crop[2], crop[3]);
  JPEGDEC_getCropArea(&jpeg, &crop[0], &crop[1], &crop[2], &crop[3]);
  *shift = ScaleShift(&jpeg, options);
  g_canvas.width = (crop[2] + (1 << *shift) - 1) >> *shift;
  g_canvas.height = (crop[3] + (1 << *shift) - 1) >> *shift;
  g_canvas.pixels =
      calloc((size_t)g_canvas.width * g_canvas.height, sizeof(uint16_t));
  g_canvas.seen = calloc((size_t)g_canvas.width * g_canvas.height, 1);
  if (g_canvas.pixels != NULL && g_canvas.seen != NULL &&
      JPEGDEC_decode(&jpeg, 0, 0, options) != 0 && g_canvas.bad == 0) {
    ok = 1;
    for (i = 0; i < g_canvas.width * g_canvas.height; i++) {
      if (g_canvas.seen[i] != 1) ok = 0;
    }
  }
  JPEGDEC_close(&jpeg);
  if (!ok) {
    fprintf(stderr, "callback decode of %dx%d output: bad blocks %d\n",
            g_canvas.width, g_canvas.height, g_canvas.bad);
  }
  return ok;
}

static void FreeCanvas(Canvas* canvas) {
  free(canvas->pixels);
  free(canvas->seen);
  memset(canvas, 0, sizeof(*canvas));
}

/* Decode into a framebuffer of the scaled crop width, padded to whole
 * 16x16 MCUs, and compare it with the whole-image decode. */
static int CheckFramebuffer(const char* name, const uint8_t* data, int size,
                            int options, const int crop[4], int shift,
                            const Canvas* whole) {
  JPEGIMAGE jpeg;
  int out_w = (crop[2] + (1 << shift) - 1) >> shift;
  int out_h = (crop[3] + (1 << shift) - 1) >> shift;
  size_t padded =
      ((size_t)out_w + 15) / 16 * 16 * (((size_t)out_h + 15) / 16 * 16);
  uint16_t* fb = calloc(padded, sizeof(uint16_t));
  int ok = 0, x, y;

  if (fb == NULL) return 0;
  if (JPEGDEC_openRAM(&jpeg, (uint8_t*)data, size, NULL) != 0) {
    JPEGDEC_setPixelType(&jpeg, JPEGD_PIXEL_RGB565_LE);
    JPEGDEC_setCropArea(&jpeg, crop[0], crop[1], crop[2], crop[3]);
    JPEGDEC_setFramebuffer(&jpeg, fb);
    ok = JPEGDEC_decode(&jpeg, 0, 0, options) != 0;
    JPEGDEC_close(&jpeg);
  }
  for (y = 0; ok && y < out_h; y++) {
    for (x = 0; x < out_w; x++) {
      uint16_t want =
          whole->pixels[(size_t)(y + (crop[1] >> shift)) * whole->width + x +
                        (crop[0] >> shift)];
      if (fb[(size_t)y * out_w + x] != want) {
        fprintf(stderr, "%s fb crop %d,%d %dx%d shift %d: (%d,%d) differs\n",
                name, crop[0], crop[1], crop[2], crop[3], shift, x, y);
        ok = 0;
        break;
      }
    }
  }
  free(fb);
  return ok;
}

static int Check(const char* name, const uint8_t* data, int size, int options,
                 int x, int y, int w, int h) {
  int none[4] = {0, 0, 0, 0}, crop[4] = {x, y, w, h};
  int shift, crop_shift, ok = 1, i, j;
  Canvas whole;

  if (!DecodeCallback(data, size, options, none, &shift)) return 0;
  whole = g_canvas;
  if (!DecodeCallback(data, size, options, crop, &crop_shift)) {
    FreeCanvas(&whole);
    FreeCanvas(&g_canvas);
    fprintf(stderr, "%s crop %d,%d %dx%d options 0x%x failed\n", name, x, y, w,
            h, options);
    return 0;
  }
  for (j = 0; ok && j < g_canvas.height; j++) {
    for (i = 0; i < g_canvas.width; i++) {
      uint16_t want =
          whole.pixels[(size_t)(j + (crop[1] >> shift)) * whole.width + i +
                       (crop[0] >> shift)];
      if (g_canvas.pixels[(size_t)j * g_canvas.width + i] != want) {
        fprintf(stderr, "%s crop %d,%d %dx%d shift %d: (%d,%d) differs\n", name,
                crop[0], crop[1], crop[2], crop[3], shift, i, j);
        ok = 0;
        break;
      }
    }
  }
  FreeCanvas(&g_canvas);
  if (ok) ok = CheckFramebuffer(name, data, size, options, crop, shift, &whole);
  FreeCanvas(&whole);
  return ok;
}

int main(void) {
  static const int kOptions[] = {0, JPEG_SCALE_HALF, JPEG_SCALE_QUARTER,
                                 JPEG_SCALE_EIGHTH};
  static const int kTulips[][4] = {
      {320, 240, 160, 160}, {16, 32, 256, 64}, {600, 440, 64, 64}};
  static const int kZebra[][4] = {{8, 16, 64, 40}, {160, 120, 160, 120}};
  static const int kProgressive[][4] = {
      {32, 16, 32, 32}, {0, 0, 64, 64}, {16, 48, 48, 16}};
  size_t o, r;
  int ok = 1;

  for (o = 0; o < sizeof(kOptions) / sizeof(kOptions[0]); o++) {
    for (r = 0; r < sizeof(kTulips) / sizeof(kTulips[0]); r++) {
      ok &= Check("tulips", tulips, (int)sizeof(tulips), kOptions[o],
                  kTulips[r][0], kTulips[r][1], kTulips[r][2], kTulips[r][3]);
    }
    for (r = 0; r < sizeof(kZebra) / sizeof(kZebra[0]); r++) {
      ok &= Check("zebra", zebra, (int)sizeof(zebra), kOptions[o], kZebra[r][0],
                  kZebra[r][1], kZebra[r][2], kZebra[r][3]);
    }
  }
  /* Progressive images decode at 1/8 whatever scale is requested. */
  for (o = 0; o < sizeof(kOptions) / sizeof(kOptions[0]); o++) {
    for (r = 0; r < sizeof(kProgressive) / sizeof(kProgressive[0]); r++) {
      ok &= Check("progressive", progressive_test_image,
                  (int)sizeof(progressive_test_image), kOptions[o],
                  kProgressive[r][0], kProgressive[r][1], kProgressive[r][2],
                  kProgressive[r][3]);
    }
  }
  if (!ok) return 1;
  printf("Cropped and scaled output matches the scaled whole image\n");
  return 0;
}
