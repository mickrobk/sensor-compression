
#include "compress.h"

#include <inttypes.h>
#include <stdint.h>

#include <cstddef>
#include <cstdio>
#include <limits>

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
        printf("NEGATIVE OVERFLOW diff=%" PRIu64 "\n", positive_diff);
        return false;
      }
    } else {
      uint64_t positive_diff = last - decoded[i];
      if (__builtin_expect(positive_diff < std::numeric_limits<int32_t>::max(), true)) {
        encoded[i] = -static_cast<int32_t>(positive_diff);
      } else {
        printf("POSITIVE OVERFLOW diff=%" PRIu64 "\n", positive_diff);
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
      printf("OVERFLOW last=%" PRIu64 "encoded=%d\n", last, encoded[i]);
      return false;
    }
    if (__builtin_expect(encoded[i] < 0 && -encoded[i] > last, false)) {
      printf("UNDERFLOW last=%" PRIu64 " encoded=%d\n", last, encoded[i]);
      return false;
    }
    decoded[i] = last + encoded[i];
    last = decoded[i];
  }
  return true;
}

std::optional<size_t> RleEncode(size_t min_run_length, uint64_t *input, size_t count,
                                uint64_t *out) {
  if (count == 0) return 0;
  size_t i = 0;
  size_t out_count = 0;
  auto write = [&](uint64_t value) {
    // printf("RLE write %llx to out_count=%zu\n", value, out_count);
    out[out_count++] = value;
  };
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
      write((run_length << 1) | 1);
      write(input[i] << 1 | 0);
    } else {
      auto value = input[i] << 1 | 0;
      for (size_t i = 0; i < run_length; i++) {
        write(value);
      }
    }
    i += run_length;
  }
  return out_count;
}

bool RleDecode(uint64_t *input, size_t count, uint64_t *out, size_t out_size) {
  if (count == 0) return out_size == 0;
  size_t i = 0;
  size_t out_count = 0;
  auto write_out = [&](uint64_t value) {
    if (out_count >= out_size) {
      printf("RLE decode overrun, out_count=%zu out_size=%zu\n", out_count, out_size);
      return false;
    }
    // printf("out value %d at %zu\n", value, out_count);
    out[out_count++] = value;
    return true;
  };
  while (i < count) {
    // printf("in value %llx at %zu\n", input[i], i);
    if ((input[i] & 0x01) == 0) {
      if (!write_out(input[i++] >> 1)) return false;
    } else {
      size_t run_length = (input[i++] >> 1);
      if (run_length == 0) {
        printf("RLE decode failure, run of length %zu at pos %zu\n", run_length, i);
        return false;
      }

      if (i >= count) {
        printf("RLE decode failure, looking for run of length %zu at pos %zu\n", run_length, i);
        return false;
      }
      if ((input[i] & 0x01) != 0) {
        printf(
            "RLE decode failure, looking for run of length %zu, but got another rle token %llx\n",
            run_length, input[i]);
        return false;
      }
      auto value = input[i++] >> 1;
      for (size_t j = 0; j < run_length; j++) {
        if (!write_out(value)) return false;
      }
    }
  }
  if (out_count != out_size) {
    printf("RLE decode failure, out_count=%zu out_size=%zu\n", out_count, out_size);
    return false;
  }
  return true;
}

}  // namespace sensor_compress