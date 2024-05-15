#pragma once
#include <chrono>

namespace sensor_compress {
using steady_time_point_t = std::chrono::time_point<std::chrono::steady_clock>;

inline uint64_t ToMs(steady_time_point_t tp) {
  auto mills = std::chrono::duration_cast<std::chrono::milliseconds>(tp.time_since_epoch());
  return static_cast<uint64_t>(mills.count());
}
inline steady_time_point_t FromMs(uint64_t ms) {
  auto t = std::chrono::duration_cast<steady_time_point_t::duration>(std::chrono::milliseconds(ms));
  return steady_time_point_t(t);
}
}  // namespace sensor_compress