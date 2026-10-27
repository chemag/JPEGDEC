/*
 * SPDX-License-Identifier: Apache-2.0
 *
 * Reject marker segments whose lengths run past the end of the input
 * instead of seeking beyond it repeatedly. Build with UBSan signed overflow
 * checking.
 */

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "../src/JPEGDEC.h"

#define FILE_SIZE 70002

int main(void) {
  static uint8_t jpeg_data[FILE_SIZE];
  JPEGIMAGE jpeg;
  int result = 1;

  /* SOI followed only by APP2 headers that each claim 65535 bytes. */
  jpeg_data[0] = 0xff;
  jpeg_data[1] = 0xd8;
  for (size_t i = 2; i + 4 <= FILE_SIZE; i += 4) {
    jpeg_data[i] = 0xff;
    jpeg_data[i + 1] = 0xe2;
    jpeg_data[i + 2] = 0xff;
    jpeg_data[i + 3] = 0xff;
  }

  if (JPEGDEC_openRAM(&jpeg, jpeg_data, FILE_SIZE, NULL) != 0) {
    fprintf(stderr, "truncated segment was accepted\n");
  } else if (JPEGDEC_getLastError(&jpeg) != JPEG_DECODE_ERROR) {
    fprintf(stderr, "unexpected error: %d\n", JPEGDEC_getLastError(&jpeg));
  } else {
    result = 0;
  }
  JPEGDEC_close(&jpeg);
  if (result == 0) printf("Segment past end of input rejected\n");
  return result;
}
