/*
 * SPDX-License-Identifier: Apache-2.0
 *
 * Decode a corrupt image whose DC differences accumulate beyond the int16
 * range. The DC predictor used to grow without bound and overflow int when
 * multiplied by the quantizer. Build with UBSan signed-overflow checking.
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "../src/JPEGDEC.h"
#include "dc_predictor_image.h"

static int Draw(JPEGDRAW* draw) {
  return draw != NULL && draw->pPixels != NULL;
}

/* Decoding may fail; it must not overflow. */
static int Decode(int mode) {
  JPEGIMAGE jpeg;
  uint8_t *y = NULL, *c = NULL;
  int width, height, chroma_width, chroma_height;

  if (JPEG_openRAM(&jpeg, (uint8_t*)dc_predictor_image,
                      (int)sizeof(dc_predictor_image), Draw) == 0) {
    fprintf(stderr, "open failed: %d\n", JPEG_getLastError(&jpeg));
    return 0;
  }
  width = JPEG_getWidth(&jpeg);
  height = JPEG_getHeight(&jpeg);
  chroma_width = (width + 1) / 2;
  chroma_height = (height + 1) / 2;
  if (mode == 0) {
    (void)JPEG_decode(&jpeg, 0, 0, 0);
  } else {
    y = malloc((size_t)width * (size_t)height);
    c = malloc((size_t)chroma_width * 2U * (size_t)chroma_height);
    if (y == NULL || c == NULL) {
      fprintf(stderr, "allocation failed\n");
      JPEG_close(&jpeg);
      free(y);
      free(c);
      return 0;
    }
    if (mode == 1) {
      (void)JPEG_decodeNV12(&jpeg, y, width, c, chroma_width * 2);
    } else {
      (void)JPEG_decodeYUV420P(&jpeg, y, width, c, chroma_width,
                                  c + (size_t)chroma_width * chroma_height,
                                  chroma_width);
    }
  }
  JPEG_close(&jpeg);
  free(y);
  free(c);
  return 1;
}

int main(void) {
  if (!Decode(0) || !Decode(1) || !Decode(2)) return 1;
  printf("Corrupt DC differences decoded without overflow\n");
  return 0;
}
