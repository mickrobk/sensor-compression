#pragma once

#include <cstdint>
#include <string>

#include "correction.h"

namespace sensor_compress {

class Sensor {
 public:
 private:
  uint64_t uuid_;
  std::string name_;
  std::string unit_;
  CombinedCorrection correction_;
};

}  // namespace sensor_compress