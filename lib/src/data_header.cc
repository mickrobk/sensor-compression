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
    : DataHeader("unknown", min, max, resolution_bits) {}
DataHeader::DataHeader(std::string_view name, uint min, uint max, uint resolution_bits)
    : name(name), min(min), max(max), resolution_bits(resolution_bits) {
  SetTimeToNow();
}

void DataHeader::SetTimeToNow() {
  start_time_utc = std::chrono::system_clock::now();
  start_time_steady = std::chrono::steady_clock::now();
}
DataHeader::DataHeader(std::string_view name, uint min, uint max)
    : DataHeader(name, min, max, CountBits(max - min)) {}
DataHeader::DataHeader(uint min, uint max) : DataHeader(min, max, CountBits(max - min)) {}

}  // namespace sensor_compress
