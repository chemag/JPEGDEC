/*
 * SPDX-License-Identifier: Apache-2.0
 *
 * Build Huffman lookup tables containing short codes. Build with UBSan
 * shift checking to detect negative shift counts while classifying codes.
 */

#include <stdint.h>
#include <stdio.h>

#include "../src/JPEGDEC.h"
#include "../test_images/tulips.h"
#include "../test_images/zebra.h"
#include "progressive_test_image.h"

static int Draw(JPEGDRAW* draw) {
  return draw != NULL && draw->pPixels != NULL;
}

static int Decode(const uint8_t* image, size_t size, const char* name) {
  JPEGIMAGE jpeg;
  int result = 0;

  if (JPEGDEC_openRAM(&jpeg, (uint8_t*)image, (int)size, Draw) == 0) {
    fprintf(stderr, "%s: open failed: %d\n", name, JPEGDEC_getLastError(&jpeg));
  } else if (JPEGDEC_decode(&jpeg, 0, 0, 0) == 0) {
    fprintf(stderr, "%s: decode failed: %d\n", name,
            JPEGDEC_getLastError(&jpeg));
  } else {
    result = 1;
  }
  JPEGDEC_close(&jpeg);
  return result;
}

int main(void) {
  if (!Decode(tulips, sizeof(tulips), "4:2:0 baseline") ||
      !Decode(zebra, sizeof(zebra), "4:4:4 baseline") ||
      !Decode(progressive_test_image, sizeof(progressive_test_image),
              "progressive")) {
    return 1;
  }
  printf("Huffman tables built without invalid shifts\n");
  return 0;
}
