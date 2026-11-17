/*
 * SPDX-License-Identifier: Apache-2.0
 *
 * Reject frame headers with zero components or more components than the
 * decoder's four-entry component table. Build with UBSan bounds checking.
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../src/JPEGDEC.h"
#include "../test_images/tulips.h"

static int CheckCount(uint8_t components, int expected_error) {
  uint8_t* jpeg_data = malloc(sizeof(tulips));
  JPEGIMAGE jpeg;
  size_t i;
  int result = 0;

  if (jpeg_data == NULL) return 0;
  memcpy(jpeg_data, tulips, sizeof(tulips));
  for (i = 2; i + 9 < sizeof(tulips); ++i) {
    if (jpeg_data[i] == 0xff && jpeg_data[i + 1] == 0xc0) break;
  }
  if (i + 9 >= sizeof(tulips)) {
    fprintf(stderr, "fixture has no SOF0 segment\n");
  } else {
    jpeg_data[i + 9] = components; /* Nf follows marker, Lf, P, Y, and X */
    if (JPEGDEC_openRAM(&jpeg, jpeg_data, (int)sizeof(tulips), NULL) != 0) {
      fprintf(stderr, "%u components were accepted\n", components);
    } else if (JPEGDEC_getLastError(&jpeg) != expected_error) {
      fprintf(stderr, "%u components: unexpected error %d\n", components,
              JPEGDEC_getLastError(&jpeg));
    } else {
      result = 1;
    }
    JPEGDEC_close(&jpeg);
  }
  free(jpeg_data);
  return result;
}

int main(void) {
  if (!CheckCount(0, JPEG_DECODE_ERROR) || !CheckCount(5, JPEG_DECODE_ERROR) ||
      !CheckCount(255, JPEG_DECODE_ERROR)) {
    return 1;
  }
  printf("Invalid frame component counts rejected\n");
  return 0;
}
