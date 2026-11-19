---
type: "developer-guide"
title: "Opt-in JPEGDEC fuzzing"
description: "Build and run standalone JPEGDEC libFuzzer targets explicitly."
tags: [jpegdec, fuzzing, libfuzzer, sanitizers]
author: "Chema Gonzalez"
generated_by: "Metamate"
date: "2026-10-26"
---

# 1. Scope

This directory is a standalone host-only CMake project inspired by h265nal's
opt-in libFuzzer build. It is not referenced by the normal ESP-IDF or Linux
builds. `BUILD_CLANG_FUZZER` defaults to `OFF`, even when configuring this
project directly. Nothing builds or runs fuzzers unless explicitly enabled.

The three targets exercise the portable C API:

* `jpegdec_header_fuzzer`: Image opening, JPEG/Exif header parsing, and
  metadata getters, including malformed and truncated input.
* `jpegdec_decode_fuzzer`: RGB565 in both byte orders, RGB8888, grayscale,
  block callbacks, unscaled framebuffer output, and 1/2, 1/4, 1/8 scaling.
* `jpegdec_yuv_fuzzer`: Planar YUV420 and NV12, including padded pitches and
  odd image dimensions.

The decoder and harnesses are instrumented with libFuzzer coverage,
AddressSanitizer, and UndefinedBehaviorSanitizer. UBSan errors are fatal so
libFuzzer preserves reproducers. The one exception is listed in
`../linux/ubsan_ignorelist.txt`: the signed-overflow check is compiled out of
`JPEGIDCT()`, whose fixed-point multiplies overflow on corrupt coefficients
and produce garbage pixels by design. Signed overflow anywhere else stays
fatal. `NO_SIMD` selects the portable path used by constrained targets;
architecture-specific SIMD and C++ wrappers are not covered. Deliberate
unsigned arithmetic is not treated as undefined behavior.

# 2. Explicit build

Keep build products outside the source checkout. From the JPEGDEC repository,
choose an external build path, for example
`build="$HOME/workspace/jpegdec-fuzz"`, then run:

* `cmake -S fuzz -B "$build" -G Ninja -DBUILD_CLANG_FUZZER=ON`
  with `-DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++`
  and `-DCMAKE_BUILD_TYPE=RelWithDebInfo` on the same command.
* `cmake --build "$build" --parallel 2`.

Clang and its libFuzzer/sanitizer runtimes are required. GCC is rejected only
when fuzzing is enabled. No packages, corpora, or submodules are fetched by
CMake. Sanitizer flags are target-local and do not change normal builds.

# 3. Seeds and operation

Generate seeds only when requested:

* `python3 fuzz/seed_corpus.py --output-dir "$build/seeds"`.

The script exports the existing baseline, progressive, and odd-dimension
fixtures. It does not download data or modify source test images. Use a
separate, writable corpus directory per target so campaigns do not race:

* `mkdir -p "$build/corpus/header" "$build/artifacts/header"`.
* Copy the seed files into `"$build/corpus/header/"`.
* Run `"$build/jpegdec_header_fuzzer" "$build/corpus/header"`
  with `-runs=1000 -max_len=1048576 -timeout=10 -rss_limit_mb=256`
  and `-artifact_prefix="$build/artifacts/header/"` on the same command.

Repeat with `decode` or `yuv` in the target and directory names. Use bounded
runs for smoke testing; start long campaigns explicitly. The harnesses cap
input size at 1 MiB. Decode targets accept positive dimensions up to 4096
per side and at most 1,048,576 pixels. These caps bound allocations and work;
they are not codec-format limits and do not cover larger images.

Trailer bytes select formats, scaling, pitches, and callback/framebuffer
mode without removing any JPEG bytes. Appending a small trailer after EOI
can exercise multiple options with the same valid seed. Files remain raw
JPEG inputs rather than a custom prefixed container.

Replay a finding with the matching executable and the artifact path, for
example `"$build/jpegdec_yuv_fuzzer" "$build/artifacts/yuv/crash-..."`.
A sanitizer finding is not silently ignored or converted into success.
Minimize and fix decoder findings as separate patches with targeted tests;
adding these harnesses does not establish that the decoder is fuzz-clean.

# 4. Build-gate regression test

Run `python3 fuzz/test_build_options.py --work-dir "$build/gate-tests"`.
It checks that default and explicit-OFF builds contain no fuzz targets,
decoder objects, or sanitizer flags, and that an explicit-ON GCC build fails
with the Clang requirement. It does not execute a fuzzing campaign.
