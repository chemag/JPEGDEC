#!/usr/bin/env python3
# SPDX-License-Identifier: Apache-2.0
"""Write small synthetic baseline JPEG fixtures as C headers.

Each image repeats one luma block pattern: a DC level plus one vertical
frequency, so every 8-pixel row differs while every block is identical. That
makes right-edge output directly comparable with the first block. Quantization
and Huffman tables are copied from test_images/tulips.h.

Usage: make_test_jpeg.py <name> <width> <height> <luma_sampling_hex>
  e.g. make_test_jpeg.py edge_444_image 20 8 11 > edge_444_image.h
"""

import argparse
import pathlib
import re
import sys

ZIGZAG_VERTICAL_1 = 2  # zigzag index of coefficient (v=1, u=0)


def fixture_bytes():
    path = pathlib.Path(__file__).resolve().parent.parent / "test_images/tulips.h"
    text = path.read_text()
    body = text[text.index("{") :]
    return bytes(int(x, 16) for x in re.findall(r"0x([0-9a-fA-F]{2})", body))


def segments(data, marker):
    """Return the raw bytes (marker included) of every segment of a type."""
    out, i = [], 2
    while i + 4 <= len(data):
        if data[i] != 0xFF:
            break
        code = data[i + 1]
        length = int.from_bytes(data[i + 2 : i + 4], "big")
        if code == marker:
            out.append(data[i : i + 2 + length])
        if code == 0xDA:
            break
        i += 2 + length
    return out


def huffman_tables(dht_segments):
    """Map (class, id) to {symbol: (code, length)} from DHT segments."""
    tables = {}
    for seg in dht_segments:
        p = 4
        while p < len(seg):
            tc_th = seg[p]
            counts = seg[p + 1 : p + 17]
            symbols = seg[p + 17 : p + 17 + sum(counts)]
            codes, code, k = {}, 0, 0
            for length in range(1, 17):
                for _ in range(counts[length - 1]):
                    codes[symbols[k]] = (code, length)
                    code += 1
                    k += 1
                code <<= 1
            tables[(tc_th >> 4, tc_th & 15)] = codes
            p += 17 + sum(counts)
    return tables


class BitWriter:
    def __init__(self):
        self.out = bytearray()
        self.acc = 0
        self.bits = 0

    def put(self, value, length):
        for shift in range(length - 1, -1, -1):
            self.acc = (self.acc << 1) | ((value >> shift) & 1)
            self.bits += 1
            if self.bits == 8:
                self.out.append(self.acc)
                if self.acc == 0xFF:
                    self.out.append(0x00)  # byte stuffing
                self.acc = 0
                self.bits = 0

    def flush(self):
        while self.bits:
            self.put(1, 1)  # pad with ones
        return bytes(self.out)


def magnitude(value):
    size = abs(value).bit_length()
    bits = value if value >= 0 else value + (1 << size) - 1
    return size, bits


def encode_block(writer, coeffs, pred, dc_table, ac_table):
    diff = coeffs[0] - pred
    size, bits = magnitude(diff)
    writer.put(*dc_table[size])
    writer.put(bits, size)
    run = 0
    last = max((k for k in range(1, 64) if coeffs[k]), default=0)
    for k in range(1, last + 1):
        if coeffs[k] == 0:
            run += 1
            continue
        while run > 15:
            writer.put(*ac_table[0xF0])
            run -= 16
        size, bits = magnitude(coeffs[k])
        writer.put(*ac_table[(run << 4) | size])
        writer.put(bits, size)
        run = 0
    if last < 63:
        writer.put(*ac_table[0x00])  # EOB
    return coeffs[0]


def make_jpeg(width, height, luma_sampling):
    src = fixture_bytes()
    dqt = segments(src, 0xDB)
    dht = segments(src, 0xC4)
    tables = huffman_tables(dht)
    h, v = luma_sampling >> 4, luma_sampling & 15
    comps = [(1, luma_sampling, 0, 0), (2, 0x11, 1, 1), (3, 0x11, 1, 1)]
    luma = [0] * 64
    luma[0] = 0
    luma[ZIGZAG_VERTICAL_1] = 40
    chroma = [0] * 64

    out = bytearray(b"\xff\xd8")
    for seg in dqt:
        out += seg
    sof = bytearray([8]) + height.to_bytes(2, "big") + width.to_bytes(2, "big")
    sof.append(len(comps))
    for cid, samp, qt, _ in comps:
        sof += bytes([cid, samp, qt])
    out += b"\xff\xc0" + (len(sof) + 2).to_bytes(2, "big") + sof
    for seg in dht:
        out += seg
    sos = bytearray([len(comps)])
    for cid, _, _, tbl in comps:
        sos += bytes([cid, (tbl << 4) | tbl])
    sos += bytes([0, 63, 0])
    out += b"\xff\xda" + (len(sos) + 2).to_bytes(2, "big") + sos

    writer = BitWriter()
    preds = [0, 0, 0]
    mcus_x = -(-width // (8 * h))
    mcus_y = -(-height // (8 * v))
    for _ in range(mcus_x * mcus_y):
        for c, (_, samp, _, tbl) in enumerate(comps):
            blocks = (samp >> 4) * (samp & 15)
            coeffs = luma if c == 0 else chroma
            for _ in range(blocks):
                preds[c] = encode_block(
                    writer, coeffs, preds[c], tables[(0, tbl)], tables[(1, tbl)]
                )
    out += writer.flush()
    out += b"\xff\xd9"
    return bytes(out)


def main():
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    parser.add_argument("name")
    parser.add_argument("width", type=int)
    parser.add_argument("height", type=int)
    parser.add_argument("luma_sampling", type=lambda s: int(s, 16))
    args = parser.parse_args()
    data = make_jpeg(args.width, args.height, args.luma_sampling)
    guard = args.name.upper() + "_H"
    print("/*")
    print(" * SPDX-License-Identifier: Apache-2.0")
    print(" *")
    print(
        " * Generated by make_test_jpeg.py %s %d %d %x"
        % (args.name, args.width, args.height, args.luma_sampling)
    )
    print(" */")
    print()
    print("#ifndef %s" % guard)
    print("#define %s" % guard)
    print()
    print("#include <stdint.h>")
    print()
    print("static const uint8_t %s[] = {" % args.name)
    for i in range(0, len(data), 12):
        print("    " + " ".join("0x%02x," % b for b in data[i : i + 12]))
    print("};")
    print()
    print("#endif  /* %s */" % guard)
    return 0


if __name__ == "__main__":
    sys.exit(main())
