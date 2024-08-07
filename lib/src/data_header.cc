#include "data_header.h"

namespace sensor_compress {

static inline unsigned int CountBits(uint v) {
  unsigned int count = 0;
  while (v > 0) {
    count++;
    v >>= 1;
  }
  return count;
}

DataHeader::DataHeader() {}

DataHeader::DataHeader(uint min, uint max, uint resolution_bits)
    : min(min), max(max), resolution_bits(resolution_bits) {
  start_time_utc = std::chrono::system_clock::now();
  start_time_steady = std::chrono::steady_clock::now();
}

DataHeader::DataHeader(uint min, uint max) : DataHeader(min, max, CountBits(max - min)) {}

}  // namespace sensor_compress
