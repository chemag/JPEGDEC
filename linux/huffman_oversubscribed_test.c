/*
 * SPDX-License-Identifier: Apache-2.0
 *
 * Reject Huffman tables that declare more codes of a length than fit in that
 * many bits. The table builder used to place those codes past the end of the
 * lookup tables. Build with AddressSanitizer.
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../src/JPEGDEC.h"
#include "../test_images/tulips.h"

/* Add delta[k] to the count of (k+1)-bit codes in the DHT segment defining
 * table_class (Tc/Th), keeping the total so the segment still parses. */
static int CheckOversubscribed(uint8_t table_class, const int delta[16],
                               const char* name) {
  uint8_t* jpeg_data = malloc(sizeof(tulips));
  JPEGIMAGE jpeg;
  size_t at = 0;
  int result = 0;

  if (jpeg_data == NULL) return 0;
  memcpy(jpeg_data, tulips, sizeof(tulips));
  for (size_t i = 2; i + 4 < sizeof(tulips); ++i) {
    if (jpeg_data[i] == 0xff && jpeg_data[i + 1] == 0xc4 &&
        jpeg_data[i + 4] == table_class) {
      at = i;
      break;
    }
  }
  if (at == 0) {
    fprintf(stderr, "%s: fixture has no such DHT segment\n", name);
    free(jpeg_data);
    return 0;
  }
  /* DHT: marker, Lh (2), Tc/Th, then the 16 code counts. */
  for (int k = 0; k < 16; ++k) jpeg_data[at + 5 + k] += (uint8_t)delta[k];

  if (JPEG_openRAM(&jpeg, jpeg_data, (int)sizeof(tulips), NULL) != 0) {
    fprintf(stderr, "%s: over-subscribed table was accepted\n", name);
  } else if (JPEG_getLastError(&jpeg) != JPEG_UNSUPPORTED_FEATURE) {
    fprintf(stderr, "%s: unexpected error %d\n", name,
            JPEG_getLastError(&jpeg));
  } else {
    result = 1;
  }
  JPEG_close(&jpeg);
  free(jpeg_data);
  return result;
}

int main(void) {
  /* 42 two-bit codes where at most 4 fit, in the last AC table. */
  static const int kAc[16] = {0, 40, 0, 0, 0, 0, 0, 0,
                              0, 0,  0, 0, 0, 0, 0, -40};
  /* 12 two-bit codes in the second DC table, all no longer than 6 bits so
   * no other length check applies. */
  static const int kDc[16] = {0,  9,  -1, -1, -1, -1, -1, -1,
                              -1, -1, -1, 0,  0,  0,  0,  0};

  if (!CheckOversubscribed(0x11, kAc, "AC table 1") ||
      !CheckOversubscribed(0x01, kDc, "DC table 1")) {
    return 1;
  }
  printf("Over-subscribed Huffman tables rejected\n");
  return 0;
}
