
#include "compress.h"

#include <stdint.h>

#include <cstddef>
#include <cstdio>

namespace sensor_compress {
bool DeltaEncode(uint64_t *decoded, size_t count, uint64_t *initial, int32_t *encoded) {
  if (count == 0) return true;
  uint64_t last = decoded[0];
  *initial = last;
  encoded[0] = 0;
  for (size_t i = 1; i < count; i++) {
    if (decoded[i] >= last) {
      uint64_t positive_diff = decoded[i] - last;
      if (__builtin_expect(positive_diff <= std::numeric_limits<int32_t>::max(), true)) {
        encoded[i] = static_cast<int32_t>(positive_diff);
      } else {
        printf("NEGATIVE OVERFLOW diff=%llu\n", positive_diff);
        return false;
      }
    } else {
      uint64_t positive_diff = last - decoded[i];
      if (__builtin_expect(positive_diff < std::numeric_limits<int32_t>::max(), true)) {
        encoded[i] = -static_cast<int32_t>(positive_diff);
      } else {
        printf("POSITIVE OVERFLOW diff=%llu\n", positive_diff);
        return false;
      }
    }
    last = decoded[i];
  }
  return true;
}

bool DeltaDecode(int32_t *encoded, size_t count, uint64_t initial, uint64_t *decoded) {
  if (count == 0) return true;
  uint64_t last = initial;
  for (size_t i = 0; i < count; i++) {
    if (__builtin_expect(encoded[i] > 0 && last > std::numeric_limits<uint64_t>::max() - encoded[i],
                         false)) {
      printf("OVERFLOW last=%llu encoded=%d\n", last, encoded[i]);
      return false;
    }
    if (__builtin_expect(encoded[i] < 0 && -encoded[i] > last, false)) {
      printf("UNDERFLOW last=%llu encoded=%d\n", last, encoded[i]);
      return false;
    }
    decoded[i] = last + encoded[i];
    last = decoded[i];
  }
  return true;
}

std::optional<size_t> RLEEncode(size_t min_run_length, uint64_t *input, size_t count,
                                uint64_t *out) {
  if (count == 0) return 0;
  size_t i = 0;
  size_t out_count = 0;
  while (i < count) {
    size_t run_length = 1;
    while (i + run_length < count && input[i] == input[i + run_length]) {
      run_length++;
    }
    if ((input[i] & 0xF800000000000000) != 0) {
      printf("RLE overflow %llx\n", input[i]);
      return std::nullopt;
    }
    if (run_length >= min_run_length) {
      out[out_count++] = (run_length << 1) | 1;
      out[out_count++] = input[i] << 1 | 0;
    } else {
      auto value = input[i] << 1 | 0;
      for (size_t i = 0; i < run_length; i++) {
        out[out_count++] = value;
      }
    }
    i += run_length;
  }
  return out_count;
}

std::optional<size_t> RLEDecode(uint64_t *input, size_t count, uint64_t *out) {
  if (count == 0) return 0;
  size_t i = 0;
  size_t out_count = 0;
  while (i < count) {
    if ((input[i] & 0x01) == 0) {
      out[out_count++] = input[i++] >> 1;
      continue;
    }
    size_t run_length = (input[i++] >> 1);

    if (i >= count) {
      printf("RLE decode failure, looking for run of length %zu at pos %zu\n", run_length, i);
      return false;
    }
    if ((input[i] & 0x01) != 0) {
      printf("RLE decode failure, looking for run of length %zu, but got another rle token %llx\n",
             run_length, input[i]);
      return false;
    }
    auto value = input[i++] >> 1;
    for (size_t j = 0; j < run_length; j++) {
      out[out_count++] = value;
    }
  }
  return out_count;
}

}  // namespace sensor_compress