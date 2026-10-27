/*
 * SPDX-License-Identifier: Apache-2.0
 *
 * Decode a 20-pixel-wide 4:4:4 image into an exactly sized framebuffer. The
 * last MCU column is clipped to 4 pixels; the SSE path used to store all 8
 * pixels of each row past the end of the line. Build with AddressSanitizer.
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "../src/JPEGDEC.h"
#include "edge_444_image.h"

/* Decode into a malloc'ed width * height buffer of bpp-byte pixels. */
static uint8_t* Decode(int pixel_type, int bpp, int* width, int* height) {
  JPEGIMAGE jpeg;
  uint8_t* framebuffer;

  if (JPEGDEC_openRAM(&jpeg, (uint8_t*)edge_444_image,
                      (int)sizeof(edge_444_image), NULL) == 0) {
    fprintf(stderr, "open failed: %d\n", JPEGDEC_getLastError(&jpeg));
    return NULL;
  }
  *width = JPEGDEC_getWidth(&jpeg);
  *height = JPEGDEC_getHeight(&jpeg);
  if (JPEGDEC_getSubSample(&jpeg) != 0x11 || *width != 20 || *height != 8) {
    fprintf(stderr, "unexpected fixture\n");
    JPEGDEC_close(&jpeg);
    return NULL;
  }
  framebuffer = malloc((size_t)*width * (size_t)*height * (size_t)bpp);
  if (framebuffer == NULL) {
    JPEGDEC_close(&jpeg);
    return NULL;
  }
  JPEGDEC_setPixelType(&jpeg, pixel_type);
  JPEGDEC_setFramebuffer(&jpeg, framebuffer);
  if (JPEGDEC_decode(&jpeg, 0, 0, 0) == 0) {
    fprintf(stderr, "decode failed: %d\n", JPEGDEC_getLastError(&jpeg));
    free(framebuffer);
    framebuffer = NULL;
  }
  JPEGDEC_close(&jpeg);
  return framebuffer;
}

int main(void) {
  int width, height;
  uint8_t* rgb565 = Decode(JPEGD_PIXEL_RGB565_LE, 2, &width, &height);
  uint8_t* rgb8888 = Decode(JPEGD_PIXEL_RGB8888, 4, &width, &height);
  int result = (rgb565 != NULL && rgb8888 != NULL) ? 0 : 1;

  free(rgb565);
  free(rgb8888);
  if (result == 0) printf("Clipped 4:4:4 MCUs stay within the framebuffer\n");
  return result;
}
