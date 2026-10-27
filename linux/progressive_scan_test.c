/*
 * SPDX-License-Identifier: Apache-2.0
 *
 * JPEGDEC decodes only the first scan of a progressive JPEG, which in a valid
 * file is an initial DC scan. Report AC and refinement first scans as
 * unsupported. They used to reach code that read the Huffman lookup tables
 * with the wrong layout. Build with AddressSanitizer.
 */

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "../src/JPEGDEC.h"
#include "progressive_test_image.h"

static int Draw(JPEGDRAW* draw) {
  return draw != NULL && draw->pPixels != NULL;
}

static int CheckScan(uint8_t ss, uint8_t se, uint8_t ah_al, const char* name) {
  static uint8_t jpeg_data[sizeof(progressive_test_image)];
  JPEGIMAGE jpeg;
  size_t at = 0;
  int result = 0;

  memcpy(jpeg_data, progressive_test_image, sizeof(jpeg_data));
  for (size_t i = 2; i + 4 < sizeof(jpeg_data); ++i) {
    if (jpeg_data[i] == 0xff && jpeg_data[i + 1] == 0xda) {
      at = i;
      break;
    }
  }
  if (at == 0) {
    fprintf(stderr, "%s: fixture has no SOS segment\n", name);
    return 0;
  }
  /* SOS: marker, Ls (2), Ns, Ns component selectors (2 bytes each), then
   * Ss, Se, and Ah/Al. */
  at += 5 + 2 * (size_t)jpeg_data[at + 4];
  jpeg_data[at] = ss;
  jpeg_data[at + 1] = se;
  jpeg_data[at + 2] = ah_al;

  if (JPEGDEC_openRAM(&jpeg, jpeg_data, (int)sizeof(jpeg_data), Draw) == 0) {
    fprintf(stderr, "%s: open failed: %d\n", name, JPEGDEC_getLastError(&jpeg));
  } else if (JPEGDEC_decode(&jpeg, 0, 0, 0) != 0) {
    fprintf(stderr, "%s: unsupported first scan was decoded\n", name);
  } else if (JPEGDEC_getLastError(&jpeg) != JPEG_UNSUPPORTED_FEATURE) {
    fprintf(stderr, "%s: unexpected error %d\n", name,
            JPEGDEC_getLastError(&jpeg));
  } else {
    result = 1;
  }
  JPEGDEC_close(&jpeg);
  return result;
}

int main(void) {
  if (!CheckScan(0, 0, 0x10, "DC refinement scan") ||
      !CheckScan(1, 63, 0x00, "AC scan")) {
    return 1;
  }
  printf("Unsupported progressive first scans rejected\n");
  return 0;
}
