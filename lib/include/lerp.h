
#pragma once
namespace sensor_compress {
template <typename T>
T lerp(T a, T b, T t) {
  return a + t * (b - a);
}
}  // namespace sensor_compress