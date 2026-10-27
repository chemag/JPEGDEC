/*
 * SPDX-License-Identifier: Apache-2.0
 *
 * Decode into caller-provided framebuffers that satisfy only the pixel
 * type's alignment: 2 bytes for RGB565 and 1 byte for grayscale. Build with
 * UBSan alignment checking to detect 4-byte stores through typed pointers.
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "../src/JPEGDEC.h"
#include "../test_images/tulips.h"

static int Decode(int pixel_type, size_t bpp, size_t base_offset,
                  const char* name) {
  JPEGIMAGE jpeg;
  uint8_t* storage = NULL;
  int result = 0;

  if (JPEGDEC_openRAM(&jpeg, (uint8_t*)tulips, (int)sizeof(tulips), NULL) ==
      0) {
    fprintf(stderr, "%s: open failed: %d\n", name, JPEGDEC_getLastError(&jpeg));
    return 0;
  }
  storage = calloc(
      (size_t)JPEGDEC_getWidth(&jpeg) * (size_t)JPEGDEC_getHeight(&jpeg) * bpp +
          base_offset,
      1);
  if (storage == NULL) {
    fprintf(stderr, "%s: allocation failed\n", name);
  } else {
    JPEGDEC_setFramebuffer(&jpeg, storage + base_offset);
    JPEGDEC_setPixelType(&jpeg, pixel_type);
    if (JPEGDEC_decode(&jpeg, 0, 0, 0) == 0) {
      fprintf(stderr, "%s: decode failed: %d\n", name,
              JPEGDEC_getLastError(&jpeg));
    } else {
      result = 1;
    }
  }
  JPEGDEC_close(&jpeg);
  free(storage);
  return result;
}

int main(void) {
  if (!Decode(JPEGD_PIXEL_RGB565_LE, 2, 2, "RGB565 LE") ||
      !Decode(JPEGD_PIXEL_RGB565_BE, 2, 2, "RGB565 BE") ||
      !Decode(JPEGD_PIXEL_EIGHT_BIT_GRAYSCALE, 1, 1, "grayscale")) {
    return 1;
  }
  printf("Framebuffers with minimal pixel alignment passed\n");
  return 0;
}
