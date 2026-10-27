/*
 * SPDX-License-Identifier: Apache-2.0
 *
 * Report unsupported luma sampling factors as an error. They used to leave the
 * MCU size at zero and divide by it while sizing the output buffer.
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../src/JPEGDEC.h"
#include "../test_images/tulips.h"

static int Draw(JPEGDRAW* draw) {
  return draw != NULL && draw->pPixels != NULL;
}

static int CheckSampling(uint8_t sampling) {
  uint8_t* jpeg_data = malloc(sizeof(tulips));
  JPEGIMAGE jpeg;
  size_t i;
  int result = 0;

  if (jpeg_data == NULL) return 0;
  memcpy(jpeg_data, tulips, sizeof(tulips));
  for (i = 2; i + 11 < sizeof(tulips); ++i) {
    if (jpeg_data[i] == 0xff && jpeg_data[i + 1] == 0xc0) break;
  }
  if (i + 11 >= sizeof(tulips)) {
    fprintf(stderr, "fixture has no SOF0 segment\n");
    free(jpeg_data);
    return 0;
  }
  /* SOF0: marker, Lf (2), P, Y (2), X (2), Nf, C1, then H1/V1. */
  jpeg_data[i + 11] = sampling;
  if (JPEGDEC_openRAM(&jpeg, jpeg_data, (int)sizeof(tulips), Draw) == 0) {
    fprintf(stderr, "0x%02x: open failed: %d\n", sampling,
            JPEGDEC_getLastError(&jpeg));
  } else if (JPEGDEC_decode(&jpeg, 0, 0, 0) != 0) {
    fprintf(stderr, "0x%02x: unsupported sampling was decoded\n", sampling);
  } else if (JPEGDEC_getLastError(&jpeg) != JPEG_UNSUPPORTED_FEATURE) {
    fprintf(stderr, "0x%02x: unexpected error %d\n", sampling,
            JPEGDEC_getLastError(&jpeg));
  } else {
    result = 1;
  }
  JPEGDEC_close(&jpeg);
  free(jpeg_data);
  return result;
}

int main(void) {
  if (!CheckSampling(0x13) || !CheckSampling(0x44)) return 1;
  printf("Unsupported sampling factors rejected\n");
  return 0;
}
