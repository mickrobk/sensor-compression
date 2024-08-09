#pragma once

#include <cstdint>
#include <vector>

namespace sensor_compress {

struct CompressedDataFrame {
  std::vector<uint64_t> side_channel;
  std::vector<uint8_t> values;
  std::vector<uint8_t> times;
};

}  // namespace sensor_compress
