#pragma once

#include <vector>
#include <cstdint>

namespace sensor_compress {

template <typename Allocator = std::allocator<uint8_t>>
struct CompressedDataFrame {
  std::vector<uint64_t, Allocator> side_channel;
  std::vector<uint8_t, Allocator> values;
  std::vector<uint8_t, Allocator> times;
};

}  // namespace sensor_compress
