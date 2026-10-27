#!/usr/bin/env python3
# SPDX-License-Identifier: Apache-2.0
"""Explicitly export existing JPEG test images as a libFuzzer seed corpus."""

import argparse
from pathlib import Path
import re


def extract_array(path: Path) -> bytes:
    text = path.read_text()
    match = re.search(r"\[\]\s*(?:PROGMEM\s*)?=\s*\{(.*?)\};", text, re.S)
    if match is None:
        raise ValueError(f"No byte array found in {path}")
    return bytes(int(value, 16) for value in re.findall(r"0x([0-9a-fA-F]{2})\b", match[1]))


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--output-dir", type=Path, required=True)
    args = parser.parse_args()
    root = Path(__file__).resolve().parent.parent
    seeds = {
        "tulips.jpg": extract_array(root / "test_images/tulips.h"),
        "zebra.jpg": extract_array(root / "test_images/zebra.h"),
        "progressive.jpg": extract_array(root / "linux/progressive_test_image.h"),
        "odd_dimensions.jpg": (root / "demo.jpg").read_bytes(),
    }
    args.output_dir.mkdir(parents=True, exist_ok=True)
    for name, data in seeds.items():
        if not data.startswith(b"\xff\xd8") or not data.endswith(b"\xff\xd9"):
            raise ValueError(f"Invalid JPEG seed: {name}")
        (args.output_dir / name).write_bytes(data)
        print(f"{name}: {len(data)} bytes")


if __name__ == "__main__":
    main()
