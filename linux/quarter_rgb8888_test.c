/*
 * SPDX-License-Identifier: Apache-2.0
 *
 * Decode a 1x2-sampled image at 1/4 scale to RGB8888. The first pixel of each
 * MCU used to be written over the local output pointer instead of the pixel
 * buffer, so the following writes went to a corrupted address. Build with
 * AddressSanitizer.
 */

#include <stdint.h>
#include <stdio.h>

#include "../src/JPEGDEC.h"
#include "quarter_rgb8888_image.h"

static int Draw(JPEGDRAW* draw) {
  return draw != NULL && draw->pPixels != NULL;
}

int main(void) {
  JPEGIMAGE jpeg;

  if (JPEG_openRAM(&jpeg, (uint8_t*)quarter_rgb8888_image,
                      (int)sizeof(quarter_rgb8888_image), Draw) == 0) {
    fprintf(stderr, "open failed: %d\n", JPEG_getLastError(&jpeg));
    return 1;
  }
  if (JPEG_getSubSample(&jpeg) != 0x12) {
    fprintf(stderr, "fixture is not 1x2-sampled\n");
    JPEG_close(&jpeg);
    return 1;
  }
  JPEG_setPixelType(&jpeg, RGB8888);
  JPEG_setMaxOutputSize(&jpeg, 2);
  /* The corrupt scan may fail to decode; it must not write out of bounds. */
  (void)JPEG_decode(&jpeg, 0, 0, JPEG_SCALE_QUARTER);
  JPEG_close(&jpeg);
  printf("1x2 RGB8888 quarter-scale decode stayed in bounds\n");
  return 0;
}
