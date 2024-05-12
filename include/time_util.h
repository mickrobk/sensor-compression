#pragma once
#include <chrono>

namespace sensor_compress {
using steady_time_point_t = std::chrono::time_point<std::chrono::steady_clock>;
inline uint64_t AsMs(steady_time_point_t tp) {
  auto mills = std::chrono::duration_cast<std::chrono::milliseconds>(tp.time_since_epoch());
  return static_cast<uint64_t>(mills.count());
}
}  // namespace sensor_compress