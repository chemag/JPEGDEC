/*
 * SPDX-License-Identifier: Apache-2.0
 *
 * Reject DQT segments whose length does not cover their tables. The length is
 * unsigned, so an uncovered table used to wrap it and keep the parser reading
 * past the header buffer. Build with AddressSanitizer.
 */

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "../src/JPEGDEC.h"

#define INPUT_SIZE 4096

/* SOI, then a DQT segment with length lq holding one 8-bit table, followed by
 * zero bytes that parse as further table 0 entries. */
static int CheckLength(uint16_t lq) {
  static uint8_t jpeg_data[INPUT_SIZE];
  JPEGIMAGE jpeg;
  int result = 0;

  memset(jpeg_data, 0, sizeof(jpeg_data));
  jpeg_data[0] = 0xff;
  jpeg_data[1] = 0xd8;
  jpeg_data[2] = 0xff;
  jpeg_data[3] = 0xdb;
  jpeg_data[4] = (uint8_t)(lq >> 8);
  jpeg_data[5] = (uint8_t)lq;
  memset(&jpeg_data[7], 1, 64); /* table 0, 8-bit precision, all ones */

  if (JPEG_openRAM(&jpeg, jpeg_data, INPUT_SIZE, NULL) != 0) {
    fprintf(stderr, "Lq=%u: malformed DQT was accepted\n", lq);
  } else if (JPEG_getLastError(&jpeg) != JPEG_DECODE_ERROR) {
    fprintf(stderr, "Lq=%u: unexpected error %d\n", lq,
            JPEG_getLastError(&jpeg));
  } else {
    result = 1;
  }
  JPEG_close(&jpeg);
  return result;
}

int main(void) {
  /* 68 = 2 + 65 + 1: one byte is left after the first table. */
  if (!CheckLength(68) || !CheckLength(1) || !CheckLength(0)) return 1;
  printf("DQT segments with uncovered tables rejected\n");
  return 0;
}
