#pragma once

#include <stdint.h>

#include <algorithm>
#include <cstddef>
#include <optional>

namespace sensor_compress {

template <typename T>
size_t Simple8bEncode(T *input, size_t inputLength, uint64_t *out);

template <typename T>
size_t Simple8bDecode(uint64_t *input, size_t uncompressedLength, T *out);

// All differences between consecutive values in the input must be less than
// int32_t::max(), more than int32_t::lowest().
bool DeltaEncode(uint64_t *decoded, size_t count, uint64_t *initial, int32_t *encoded);
// All result values must be >0
bool DeltaDecode(int32_t *encoded, size_t count, uint64_t initial, uint64_t *decoded);

template <typename T>
void ZigZagEncode(T *input, size_t length);
template <typename T>
void ZigZagDecode(T *input, size_t length);

std::optional<size_t> RLEEncode(size_t min_run_length, uint64_t *input, size_t count,
                                uint64_t *out);

std::optional<size_t> RLEDecode(uint64_t *input, size_t count, uint64_t *out);

#include "compress.inl"

}  // namespace sensor_compress