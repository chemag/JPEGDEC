/*
 * SPDX-License-Identifier: Apache-2.0
 *
 * Check that RGB8888 output stores R, G, B, A in byte order, like the
 * reference C converter, and agrees with RGB565 output. The SSE 4:4:4 and
 * 4:2:0 converters and the NEON 4:2:0 converter used to store B and R
 * swapped. Runs whichever converter the build selects; the SIMD paths are
 * taken for MCUs that are not clipped.
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../src/JPEGDEC.h"
#include "../test_images/tulips.h"
#include "../test_images/zebra.h"

static void* Decode(const uint8_t* data, int size, int pixel_type, int bpp,
                    int* width, int* height) {
  JPEGIMAGE jpeg;
  void* framebuffer = NULL;

  if (JPEGDEC_openRAM(&jpeg, (uint8_t*)data, size, NULL) == 0) return NULL;
  *width = JPEGDEC_getWidth(&jpeg);
  *height = JPEGDEC_getHeight(&jpeg);
  framebuffer = calloc((size_t)*width * (size_t)*height, (size_t)bpp);
  if (framebuffer != NULL) {
    JPEGDEC_setPixelType(&jpeg, pixel_type);
    JPEGDEC_setFramebuffer(&jpeg, framebuffer);
    if (JPEGDEC_decode(&jpeg, 0, 0, 0) == 0) {
      free(framebuffer);
      framebuffer = NULL;
    }
  }
  JPEGDEC_close(&jpeg);
  return framebuffer;
}

/* Returns the number of pixels whose RGB8888 and RGB565 colors disagree. */
static long Compare(const char* name, const uint8_t* data, int size) {
  int width, height, w2, h2;
  uint16_t* rgb565 =
      Decode(data, size, JPEGD_PIXEL_RGB565_LE, 2, &width, &height);
  uint8_t* rgb8888 = Decode(data, size, JPEGD_PIXEL_RGB8888, 4, &w2, &h2);
  long i, bad = 0;

  if (rgb565 == NULL || rgb8888 == NULL || width != w2 || height != h2) {
    fprintf(stderr, "%s: decode failed\n", name);
    free(rgb565);
    free(rgb8888);
    return -1;
  }
  for (i = 0; i < (long)width * height; i++) {
    const uint8_t* p = rgb8888 + i * 4;
    int r = rgb565[i] >> 11, g = (rgb565[i] >> 5) & 63, b = rgb565[i] & 31;
    /* The 565 and 8888 converters round separately: allow one step. */
    if (abs((p[0] >> 3) - r) > 1 || abs((p[1] >> 2) - g) > 1 ||
        abs((p[2] >> 3) - b) > 1 || p[3] != 0xff) {
      if (bad == 0) {
        fprintf(stderr, "%s: pixel %ld RGBA %02x%02x%02x%02x vs RGB565 %04x\n",
                name, i, p[0], p[1], p[2], p[3], rgb565[i]);
      }
      bad++;
    }
  }
  free(rgb565);
  free(rgb8888);
  return bad;
}

int main(void) {
  long tulips_bad = Compare("tulips", tulips, (int)sizeof(tulips));
  long zebra_bad = Compare("zebra", zebra, (int)sizeof(zebra));

  if (tulips_bad != 0 || zebra_bad != 0) {
    fprintf(stderr, "mismatched pixels: tulips %ld, zebra %ld\n", tulips_bad,
            zebra_bad);
    return 1;
  }
  printf("RGB8888 output is RGBA and agrees with RGB565\n");
  return 0;
}
