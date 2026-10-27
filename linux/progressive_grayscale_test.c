/*
 * SPDX-License-Identifier: Apache-2.0
 *
 * Decode a color progressive JPEG to 8-bit grayscale. Grayscale output skips
 * the chroma coefficients, which must be parsed without being stored.
 * JPEGDEC decodes only the first (DC) scan of a progressive JPEG and returns
 * a 1/8-size image.
 */

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "../src/JPEGDEC.h"
#include "progressive_test_image.h"

#define WIDTH 64
#define HEIGHT 64
#define OUT_WIDTH (WIDTH / 8)
#define OUT_HEIGHT (HEIGHT / 8)

static uint8_t gray[OUT_WIDTH * OUT_HEIGHT];
static uint32_t pixel_count;

static int Draw(JPEGDRAW* draw) {
  const uint8_t* src = (const uint8_t*)draw->pPixels;

  if (draw->pPixels == NULL || draw->iBpp != 8 || draw->x < 0 || draw->y < 0 ||
      draw->iWidthUsed < 1 || draw->iHeight < 1 ||
      draw->x + draw->iWidthUsed > OUT_WIDTH ||
      draw->y + draw->iHeight > OUT_HEIGHT) {
    return 0;
  }
  for (int y = 0; y < draw->iHeight; ++y) {
    memcpy(&gray[(draw->y + y) * OUT_WIDTH + draw->x], &src[y * draw->iWidth],
           (size_t)draw->iWidthUsed);
  }
  pixel_count += (uint32_t)draw->iWidthUsed * (uint32_t)draw->iHeight;
  return 1;
}

int main(void) {
  JPEGIMAGE jpeg;
  uint32_t sum = 0;

  if (JPEGDEC_openRAM(&jpeg, (uint8_t*)progressive_test_image,
                      sizeof(progressive_test_image), Draw) == 0) {
    fprintf(stderr, "open failed: %d\n", JPEGDEC_getLastError(&jpeg));
    return 1;
  }
  if (JPEGDEC_getJPEGType(&jpeg) != JPEG_MODE_PROGRESSIVE ||
      JPEGDEC_getWidth(&jpeg) != WIDTH || JPEGDEC_getHeight(&jpeg) != HEIGHT) {
    fprintf(stderr, "unexpected progressive fixture\n");
    JPEGDEC_close(&jpeg);
    return 1;
  }
  JPEGDEC_setPixelType(&jpeg, JPEGD_PIXEL_EIGHT_BIT_GRAYSCALE);
  if (JPEGDEC_decode(&jpeg, 0, 0, 0) == 0) {
    fprintf(stderr, "decode failed: %d\n", JPEGDEC_getLastError(&jpeg));
    JPEGDEC_close(&jpeg);
    return 1;
  }
  JPEGDEC_close(&jpeg);

  for (size_t i = 0; i < sizeof(gray); ++i) sum += gray[i];
  if (pixel_count != OUT_WIDTH * OUT_HEIGHT || sum == 0) {
    fprintf(stderr, "unexpected output: %u pixels, sum %u\n", pixel_count, sum);
    return 1;
  }
  printf("Progressive color JPEG decoded to %ux%u grayscale\n", OUT_WIDTH,
         OUT_HEIGHT);
  return 0;
}
