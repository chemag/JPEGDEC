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

  if (JPEG_openRAM(&jpeg, (uint8_t*)tulips, (int)sizeof(tulips), NULL) ==
      0) {
    fprintf(stderr, "%s: open failed: %d\n", name, JPEG_getLastError(&jpeg));
    return 0;
  }
  storage = calloc(
      (size_t)JPEG_getWidth(&jpeg) * (size_t)JPEG_getHeight(&jpeg) * bpp +
          base_offset,
      1);
  if (storage == NULL) {
    fprintf(stderr, "%s: allocation failed\n", name);
  } else {
    JPEG_setFramebuffer(&jpeg, storage + base_offset);
    JPEG_setPixelType(&jpeg, pixel_type);
    if (JPEG_decode(&jpeg, 0, 0, 0) == 0) {
      fprintf(stderr, "%s: decode failed: %d\n", name,
              JPEG_getLastError(&jpeg));
    } else {
      result = 1;
    }
  }
  JPEG_close(&jpeg);
  free(storage);
  return result;
}

int main(void) {
  if (!Decode(RGB565_LITTLE_ENDIAN, 2, 2, "RGB565 LE") ||
      !Decode(RGB565_BIG_ENDIAN, 2, 2, "RGB565 BE") ||
      !Decode(EIGHT_BIT_GRAYSCALE, 1, 1, "grayscale")) {
    return 1;
  }
  printf("Framebuffers with minimal pixel alignment passed\n");
  return 0;
}
