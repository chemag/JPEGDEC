/*
 * SPDX-License-Identifier: Apache-2.0
 *
 * Decode a color progressive JPEG to 8-bit grayscale. Grayscale output skips
 * the chroma coefficients, which must be parsed without being stored: storing
 * them over the first block of the MCU corrupts every other pixel of the
 * even output rows. JPEGDEC decodes only the first (DC) scan of a progressive
 * JPEG and returns a 1/8-size image; the fixture is a vertical gradient, so
 * each output row is uniform.
 */

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "../src/JPEGDEC.h"
#include "jpegdec_test.h"
#include "progressive_test_image.h"

#define WIDTH 64
#define HEIGHT 64
#define OUT_WIDTH (WIDTH / 8)
#define OUT_HEIGHT (HEIGHT / 8)

static const uint8_t kExpected[OUT_WIDTH * OUT_HEIGHT] = {
    0x49, 0x49, 0x49, 0x49, 0x49, 0x49, 0x49, 0x49, 0x42, 0x42, 0x42,
    0x42, 0x42, 0x42, 0x42, 0x42, 0x3d, 0x3d, 0x3d, 0x3d, 0x3d, 0x3d,
    0x3d, 0x3d, 0x37, 0x37, 0x37, 0x37, 0x37, 0x37, 0x37, 0x37, 0x31,
    0x31, 0x31, 0x31, 0x31, 0x31, 0x31, 0x31, 0x2b, 0x2b, 0x2b, 0x2b,
    0x2b, 0x2b, 0x2b, 0x2b, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24,
    0x24, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f,
};

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

  if (JPEG_openRAM(&jpeg, (uint8_t*)progressive_test_image,
                      sizeof(progressive_test_image), Draw) == 0) {
    fprintf(stderr, "open failed: %d\n", JPEG_getLastError(&jpeg));
    return 1;
  }
  if (jpegdec_test_get_jpeg_type(&jpeg) != JPEG_MODE_PROGRESSIVE ||
      JPEG_getWidth(&jpeg) != WIDTH || JPEG_getHeight(&jpeg) != HEIGHT) {
    fprintf(stderr, "unexpected progressive fixture\n");
    JPEG_close(&jpeg);
    return 1;
  }
  JPEG_setPixelType(&jpeg, EIGHT_BIT_GRAYSCALE);
  if (JPEG_decode(&jpeg, 0, 0, 0) == 0) {
    fprintf(stderr, "decode failed: %d\n", JPEG_getLastError(&jpeg));
    JPEG_close(&jpeg);
    return 1;
  }
  JPEG_close(&jpeg);

  if (pixel_count != OUT_WIDTH * OUT_HEIGHT) {
    fprintf(stderr, "unexpected output: %u pixels\n", pixel_count);
    return 1;
  }
  for (size_t i = 0; i < sizeof(gray); ++i) {
    if (gray[i] != kExpected[i]) {
      fprintf(stderr, "pixel (%zu,%zu) is 0x%02x, expected 0x%02x\n",
              i % OUT_WIDTH, i / OUT_WIDTH, gray[i], kExpected[i]);
      return 1;
    }
  }
  printf("Progressive color JPEG decoded to %ux%u grayscale\n", OUT_WIDTH,
         OUT_HEIGHT);
  return 0;
}
