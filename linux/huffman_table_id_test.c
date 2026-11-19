/*
 * SPDX-License-Identifier: Apache-2.0
 *
 * Reject a DHT segment whose table class/destination byte is invalid. Build
 * with UBSan shift checking.
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../src/JPEGDEC.h"
#include "../test_images/tulips.h"

int main(void) {
  uint8_t* jpeg_data = malloc(sizeof(tulips));
  JPEGIMAGE jpeg;
  size_t i;
  int result = 1;

  if (jpeg_data == NULL) return 1;
  memcpy(jpeg_data, tulips, sizeof(tulips));
  for (i = 2; i + 4 < sizeof(tulips); ++i) {
    if (jpeg_data[i] == 0xff && jpeg_data[i + 1] == 0xc4) break;
  }
  if (i + 4 >= sizeof(tulips)) {
    fprintf(stderr, "fixture has no DHT segment\n");
    free(jpeg_data);
    return 1;
  }
  jpeg_data[i + 4] = 0x2e; /* Tc=2, Th=14: neither is valid */

  if (JPEG_openRAM(&jpeg, jpeg_data, (int)sizeof(tulips), NULL) != 0) {
    fprintf(stderr, "invalid DHT table id was accepted\n");
  } else if (JPEG_getLastError(&jpeg) != JPEG_DECODE_ERROR) {
    fprintf(stderr, "unexpected error: %d\n", JPEG_getLastError(&jpeg));
  } else {
    result = 0;
  }
  JPEG_close(&jpeg);
  free(jpeg_data);
  if (result == 0) printf("Invalid DHT table id rejected\n");
  return result;
}
