#pragma once

#include <stdint.h>

#include <algorithm>
#include <cstddef>
#include <optional>
#include <tl/expected.hpp>

namespace sensor_compress {

std::array<uint8_t, 8> u64_to_bytes(uint64_t x);
void u64_to_bytes(uint64_t x, uint8_t *b);
uint64_t u64_from_bytes(const uint8_t *b);
tl::expected<uint64_t, std::string> u64_from_bytes(const std::vector<uint8_t> &b);
std::vector<uint8_t> to_bytes(std::vector<uint64_t> v);
tl::expected<std::vector<uint64_t>, std::string> from_bytes(const std::vector<uint8_t> &b);

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

std::optional<size_t> RleEncode(size_t min_run_length, uint64_t *input, size_t count,
                                uint64_t *out);

bool RleDecode(uint64_t *input, size_t count, uint64_t *out, size_t out_size);

#include "compress.inl"

}  // namespace sensor_compress