/*
 * SPDX-License-Identifier: Apache-2.0
 *
 * Stop decoding when the entropy-coded data runs out. The bit reader used to
 * keep advancing past the 2 KiB VLC buffer once a refill could no longer reset
 * its offset. Build with AddressSanitizer.
 */

#include <stdint.h>
#include <stdio.h>

#include "../src/JPEGDEC.h"
#include "vlc_exhausted_image.h"

static int Draw(JPEGDRAW* draw) {
  return draw != NULL && draw->pPixels != NULL;
}

static int CheckImage(void) {
  JPEGIMAGE jpeg;
  int result = 0;

  if (JPEG_openRAM(&jpeg, (uint8_t*)vlc_exhausted_image,
                      (int)sizeof(vlc_exhausted_image), Draw) == 0) {
    fprintf(stderr, "open failed: %d\n", JPEG_getLastError(&jpeg));
  } else if (JPEG_decode(&jpeg, 0, 0, 0) != 0) {
    fprintf(stderr, "exhausted scan decoded fully\n");
  } else if (JPEG_getLastError(&jpeg) != JPEG_DECODE_ERROR) {
    fprintf(stderr, "unexpected error %d\n", JPEG_getLastError(&jpeg));
  } else {
    result = 1;
  }
  JPEG_close(&jpeg);
  return result;
}

int main(void) {
  if (!CheckImage()) return 1;
  printf("Exhausted entropy-coded data stops decoding\n");
  return 0;
}
