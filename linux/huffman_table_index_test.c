/*
 * SPDX-License-Identifier: Apache-2.0
 *
 * The decoder stores Huffman tables 0-1 only. Reject DC table 3 in a DHT
 * segment and a scan that selects DC table 3, instead of indexing past the
 * table storage. Build with UBSan bounds checking.
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

static size_t FindMarker(const uint8_t* data, size_t size, uint8_t marker) {
  for (size_t i = 2; i + 1 < size; ++i) {
    if (data[i] == 0xff && data[i + 1] == marker) return i;
  }
  return 0;
}

/* Patch one byte at offset `delta` after the first `marker` and expect the
 * open (or, with decode set, the decode) to fail as unsupported. */
static int CheckPatch(uint8_t marker, size_t delta, uint8_t value, int decode,
                      const char* name) {
  uint8_t* jpeg_data = malloc(sizeof(tulips));
  size_t at = FindMarker(tulips, sizeof(tulips), marker);
  JPEGIMAGE jpeg;
  int opened, ok, result = 0;

  if (jpeg_data == NULL || at == 0) {
    fprintf(stderr, "%s: setup failed\n", name);
    free(jpeg_data);
    return 0;
  }
  memcpy(jpeg_data, tulips, sizeof(tulips));
  jpeg_data[at + delta] = value;
  opened = JPEGDEC_openRAM(&jpeg, jpeg_data, (int)sizeof(tulips), Draw);
  ok = decode ? (opened && JPEGDEC_decode(&jpeg, 0, 0, 0) == 0) : !opened;
  if (!ok) {
    fprintf(stderr, "%s: invalid table was accepted\n", name);
  } else if (JPEGDEC_getLastError(&jpeg) != JPEG_UNSUPPORTED_FEATURE) {
    fprintf(stderr, "%s: unexpected error %d\n", name,
            JPEGDEC_getLastError(&jpeg));
  } else {
    result = 1;
  }
  JPEGDEC_close(&jpeg);
  free(jpeg_data);
  return result;
}

int main(void) {
  /* DHT: marker, Lh (2), then Tc/Th. 0x03 defines DC table 3. */
  if (!CheckPatch(0xc4, 4, 0x03, 0, "DHT defines DC table 3")) return 1;
  /* SOS: marker, Ls (2), Ns, Cs1, then Td/Ta for the first component. */
  if (!CheckPatch(0xda, 6, 0x30, 1, "scan selects DC table 3")) return 1;
  printf("Huffman tables beyond 0-1 rejected\n");
  return 0;
}
