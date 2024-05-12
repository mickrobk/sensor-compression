
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
}  // namespace sensor_compress