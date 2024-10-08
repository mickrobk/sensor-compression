#pragma once
#include <chrono>

namespace sensor_compress {
using steady_time_point_t = std::chrono::time_point<std::chrono::steady_clock>;
using utc_time_point_t = std::chrono::time_point<std::chrono::system_clock>;

inline steady_time_point_t steady_clock_now() { return std::chrono::steady_clock::now(); }
inline utc_time_point_t system_clock_now() { return std::chrono::system_clock::now(); }

inline uint64_t ToMs(utc_time_point_t tp) {
  auto mills = std::chrono::duration_cast<std::chrono::milliseconds>(tp.time_since_epoch());
  return static_cast<uint64_t>(mills.count());
}
inline utc_time_point_t UTCFromMs(uint64_t ms) {
  auto t = std::chrono::duration_cast<utc_time_point_t::duration>(std::chrono::milliseconds(ms));
  return utc_time_point_t(t);
}

inline uint64_t ToMs(steady_time_point_t tp) {
  auto mills = std::chrono::duration_cast<std::chrono::milliseconds>(tp.time_since_epoch());
  return static_cast<uint64_t>(mills.count());
}
inline steady_time_point_t SteadyFromMs(uint64_t ms) {
  auto t = std::chrono::duration_cast<steady_time_point_t::duration>(std::chrono::milliseconds(ms));
  return steady_time_point_t(t);
}
}  // namespace sensor_compress