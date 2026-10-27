/*
 * SPDX-License-Identifier: Apache-2.0
 *
 * Decode JPEG data whose markers and entropy-coded bytes start at arbitrary
 * byte offsets. Build with UBSan alignment checking to detect typed loads of
 * misaligned multi-byte values.
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../src/JPEGDEC.h"
#include "../test_images/tulips.h"
#include "progressive_test_image.h"

static int Draw(JPEGDRAW* draw) {
  return draw != NULL && draw->pPixels != NULL;
}

static int DecodeAtOffset(const uint8_t* image, size_t size, size_t offset,
                          const char* name) {
  JPEGIMAGE jpeg;
  uint8_t* storage = malloc(size + offset);
  int result = 0;

  if (storage == NULL) {
    fprintf(stderr, "%s: allocation failed\n", name);
    return 0;
  }
  memcpy(storage + offset, image, size);
  if (JPEGDEC_openRAM(&jpeg, storage + offset, (int)size, Draw) == 0) {
    fprintf(stderr, "%s+%zu: open failed: %d\n", name, offset,
            JPEGDEC_getLastError(&jpeg));
  } else if (JPEGDEC_decode(&jpeg, 0, 0, 0) == 0) {
    fprintf(stderr, "%s+%zu: decode failed: %d\n", name, offset,
            JPEGDEC_getLastError(&jpeg));
  } else {
    result = 1;
  }
  JPEGDEC_close(&jpeg);
  free(storage);
  return result;
}

int main(void) {
  for (size_t offset = 0; offset < 8; ++offset) {
    if (!DecodeAtOffset(tulips, sizeof(tulips), offset, "baseline") ||
        !DecodeAtOffset(progressive_test_image, sizeof(progressive_test_image),
                        offset, "progressive")) {
      return 1;
    }
  }
  printf("Baseline and progressive JPEGs decoded at offsets 0-7\n");
  return 0;
}
