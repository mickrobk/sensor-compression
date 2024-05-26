#pragma once

#include <cstdint>
#include <string>

#include "correction.h"
#include "data_frame.h"

namespace sensor_compress {

class Sensor {
 public:
  virtual ~Sensor() = default;
  virtual void Update() = 0;

  CombinedCorrection correction;
  DataHeader header;
  DataStream stream;
};

}  // namespace sensor_compress